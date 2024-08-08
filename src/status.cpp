// Standard includes
#include <array>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

// External includes
#include <linux/videodev2.h>
#include <linux/wireless.h>
#include <pwd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <unistd.h>

// Local includes
#include "status.hpp"

namespace sbar {

bool Cpu_state::update() {
    const char* const proc_stat_path = "/proc/stat";
    const char* const proc_stat_cpu_field = "cpu ";

    std::ifstream proc_stat{ proc_stat_path };

    std::string line;
    while (std::getline(proc_stat, line).good()) {
        auto line_c_str = std::string_view{ line.data(), line.size() };
        if (! remove_prefix(line_c_str, proc_stat_cpu_field)) {
            continue;
        }

        this->entries_ = to_integers(split<index_count>(line_c_str, ' '));

        return true;
    }

    return false;
}

size_t Cpu_state::get_total() const {
    size_t total = 0;
    for (size_t entry : this->entries_) {
        total += entry;
    }
    return total;
}

size_t Cpu_state::get_total(const std::vector<Index>& indicies) const {
    size_t total = 0;
    for (Index idx : indicies) {
        total += this->entries_.at(static_cast<size_t>(idx));
    }
    return total;
}

std::string get_cpu_percent(std::unique_ptr<Cpu_state>& cpu_state_info) {
    if (cpu_state_info == nullptr) {
        cpu_state_info = std::make_unique<Cpu_state>();
        if (! cpu_state_info->update()) {
            return sbar::error_str;
        }
        return sbar::standby_str;
    }

    std::vector<Cpu_state::Index> idle_components{ Cpu_state::Index::idle };

    auto prev_total = cpu_state_info->get_total();
    auto prev_idle = cpu_state_info->get_total(idle_components);
    auto prev_work = prev_total - prev_idle;

    if (! cpu_state_info->update()) {
        return sbar::error_str;
    }

    auto new_total = cpu_state_info->get_total();
    auto new_idle = cpu_state_info->get_total(idle_components);
    auto new_work = new_total - new_idle;

    auto total_diff = static_cast<double>(new_total - prev_total);
    auto work_diff = static_cast<double>(new_work - prev_work);

    return sprintf("%2.0f", (work_diff / total_diff) * 1e2);
}

std::string get_cpu_temperature() {
    const char* const sys_class_hwmon_path = "/sys/class/hwmon/";
    const char* const sys_class_hwmon_core_temp_name_filename = "name";
    const char* const sys_class_hwmon_core_temp_name = "coretemp";
    const char* const sys_class_hwmon_core_temp_prefix = "temp";
    const char* const sys_class_hwmon_core_temp_input_postfix = "_input";
    const char* const sys_class_hwmon_core_temp_label_postfix = "_label";
    const char* const sys_class_hwmon_core_temp_label = "Package id 0";

    for (const std::filesystem::directory_entry& hwmon_device :
      std::filesystem::directory_iterator(sys_class_hwmon_path)) {
        if (! hwmon_device.is_directory()) {
            continue;
        }

        std::string hwmon_name = get_first_line(
          hwmon_device.path() / sys_class_hwmon_core_temp_name_filename);
        if (hwmon_name != sys_class_hwmon_core_temp_name) {
            continue;
        }

        for (const std::filesystem::directory_entry& hwmon_device_file :
          std::filesystem::directory_iterator(hwmon_device)) {
            if (! hwmon_device_file.is_regular_file()) {
                continue;
            }

            const std::filesystem::path& sensor_root_path =
              hwmon_device_file.path();
            std::string_view sensor_id = sensor_root_path.filename().c_str();

            if (! remove_prefix(sensor_id, sys_class_hwmon_core_temp_prefix)) {
                continue;
            }

            if (! remove_postfix(
                  sensor_id, sys_class_hwmon_core_temp_input_postfix)) {
                continue;
            }

            std::string sensor_prefix{ sys_class_hwmon_core_temp_prefix
                + std::string{ sensor_id } };

            std::string sensor_label_path = hwmon_device.path()
              / (sensor_prefix + sys_class_hwmon_core_temp_label_postfix);
            std::string sensor_input_path = hwmon_device.path()
              / (sensor_prefix + sys_class_hwmon_core_temp_input_postfix);

            std::string sensor_label = get_first_line(sensor_label_path);
            if (sensor_label != sys_class_hwmon_core_temp_label) {
                continue;
            }

            std::string sensor_input = get_first_line(sensor_input_path);
            if (sensor_input == sbar::null_str) {
                continue;
            }

            return sprintf(
              "%.0f", static_cast<float>(std::stoull(sensor_input)) / 1e3F);
        }
    }

    return sbar::error_str;
}

bool Battery_state::add_sample(const Battery& battery) {
    // documentation for /sys/class/power_supply/:
    // https://github.com/torvalds/linux/blob/master/include/linux/power_supply.h
    // https://www.kernel.org/doc/html/latest/power/power_supply_class.html

    const char* const battery_energy_now_filename = "energy_now";

    std::string energy_now =
      get_first_line(battery.path() / battery_energy_now_filename);
    if (energy_now == sbar::null_str) {
        return false;
    }

    if (this->has_enough_samples()) {
        this->energy_remaining_.pop_front();
    }

    this->energy_remaining_.push_back(std::stoull(energy_now));

    return true;
}

bool Battery_state::has_enough_samples() const {
    return this->energy_remaining_.size() >= sample_size;
}

std::string Battery_state::get_time_remaining() const {
    if (! this->has_enough_samples()) {
        return sbar::standby_str;
    }

    size_t largest_sample = this->energy_remaining_.front();
    size_t smallest_sample = this->energy_remaining_.back();

    size_t difference = largest_sample - smallest_sample;
    if (difference == 0) {
        return sbar::error_str;
    }
    size_t sample_periods_until_empty = largest_sample / difference;
    size_t seconds_until_empty = sample_periods_until_empty * sample_size;

    const size_t seconds_per_minute = 60;
    const size_t seconds_per_hour = 3600;

    size_t minutes_until_empty = seconds_until_empty % seconds_per_minute;
    size_t hours_until_empty = seconds_until_empty / seconds_per_hour;

    return sprintf("%.2i:%.2i", hours_until_empty, minutes_until_empty);
}

Backlight::Backlight() {
    // documentation for /sys/class/backlight/:
    // https://github.com/torvalds/linux/blob/master/include/linux/backlight.h
    // https://docs.kernel.org/gpu/backlight.html

    const char* const devices_path = "/sys/class/backlight/";
    const char* const device_brightness_filename = "brightness";
    const char* const device_max_brightness_filename = "max_brightness";

    for (const std::filesystem::directory_entry& device :
      std::filesystem::directory_iterator(devices_path)) {
        if (! std::filesystem::exists(
              device.path() / device_brightness_filename)) {
            continue;
        }
        if (! std::filesystem::exists(
              device.path() / device_max_brightness_filename)) {
            continue;
        }

        this->path_ = device.path();
        this->good_ = true;
        return;
    }
}

std::string get_backlight_percent(const Backlight& backlight) {
    // documentation for /sys/class/backlight/:
    // https://github.com/torvalds/linux/blob/master/include/linux/backlight.h
    // https://docs.kernel.org/gpu/backlight.html

    const char* const battery_brightness_filename = "brightness";
    const char* const battery_max_brightness_filename = "max_brightness";

    std::string brightness =
      get_first_line(backlight.path() / battery_brightness_filename);
    if (brightness == sbar::null_str) {
        return sbar::error_str;
    }

    std::string max_brightness =
      get_first_line(backlight.path() / battery_max_brightness_filename);
    if (max_brightness == sbar::null_str) {
        return sbar::error_str;
    }

    return sprintf(
      "%.0f", std::stof(brightness) / std::stof(max_brightness) * 1e2);
}

size_t Network_data_stats::get_upload_byte_difference(
  size_t upload_byte_count) {
    size_t difference = upload_byte_count - this->upload_byte_count_;
    this->upload_byte_count_ = upload_byte_count;
    return difference;
}

size_t Network_data_stats::get_download_byte_difference(
  size_t download_byte_count) {
    size_t difference = download_byte_count - this->download_byte_count_;
    this->download_byte_count_ = download_byte_count;
    return difference;
}

std::string get_microphone_status() {
    const char* const asound_path = "/proc/asound/";
    const char* const card_prefix = "card";
    const char* const device_prefix = "pcm";
    const char* const device_postfix = "c"; // 'c' for capture
    const char* const status_filename = "status";

    size_t closed_device_count = 0;

    for (const std::filesystem::directory_entry& card :
      std::filesystem::directory_iterator(asound_path)) {
        if (! card.is_directory()) {
            continue;
        }

        std::string card_name = card.path().filename().string();
        std::string_view card_name_raw{ card_name };

        if (! remove_prefix(card_name_raw, card_prefix)) {
            continue;
        }

        for (const std::filesystem::directory_entry& device :
          std::filesystem::directory_iterator(card)) {
            if (! device.is_directory()) {
                continue;
            }

            std::string device_name = device.path().filename().string();
            std::string_view device_name_raw{ device_name };

            if (! (remove_prefix(device_name_raw, device_prefix)
                  && remove_postfix(device_name_raw, device_postfix))) {
                continue;
            }

            for (const std::filesystem::directory_entry& sub_device :
              std::filesystem::directory_iterator(device)) {
                if (! sub_device.is_directory()) {
                    continue;
                }

                std::string status = get_first_line(
                  sub_device / std::filesystem::path(status_filename));

                if (status != "closed") {
                    return "🟢";
                }

                closed_device_count++;
            }
        }
    }

    if (closed_device_count == 0) {
        return "❌";
    }

    return "🔴";
}

// class readonly_file {
//     std::FILE* file_;

//   public:
//     explicit readonly_file(const char* path) : file_(std::fopen(path,
//     "r")) {
//     }

//     readonly_file(const readonly_file&) = delete;
//     readonly_file(readonly_file&&) noexcept = default;
//     readonly_file& operator=(const readonly_file&) = delete;
//     readonly_file& operator=(readonly_file&&) noexcept = default;

//     ~readonly_file() {
//         std::fclose(this->file_);
//         // do nothing if the file fails to close.
//     }

//     [[nodiscard]] bool good() const {
//         return this->file_ == nullptr;
//     }

//     template<typename... request_t>
//     bool request(unsigned long request_type, request_t&... request) {
//         return ::sbar::request(fileno(this->file_), request_type,
//         request...);
//     }
// };

std::string get_camera_status() {
    // readonly_file file{ "/dev/video0" };

    // struct v4l2_capability capabilities {};

    // const int argp = 0;

    // struct v4l2_requestbuffers buffers {};

    // struct v4l2_queryctrl ctrl {};

    // if (! file.request(VIDIOC_QUERYCTRL, ctrl)) {
    //     return sbar::error_str;
    // }

    // // return sprintf("%s", capabilities.driver);
    // return sprintf("%s", ctrl.name);

    return sbar::null_str;
}

std::string get_user() {
    auto uid = geteuid();

    struct passwd* passwd_info = getpwuid(uid);
    if (passwd_info == nullptr) {
        return sbar::error_str;
    }

    return passwd_info->pw_name;
}

std::string get_outdated_kernel_indicator() {
    const char* const modules_path = "/usr/lib/modules/";

    utsname utsname_info{};
    if (uname(&utsname_info) != 0) {
        int err = errno;
        std::cerr << "uname(): " << std::strerror(err) << '\n';
        return sbar::error_str;
    }

    std::string running_release{ static_cast<const char*>(
      utsname_info.release) };

    std::string latest_installed_release{};

    for (const std::filesystem::directory_entry& release :
      std::filesystem::directory_iterator(modules_path)) {
        if (! release.is_directory()) {
            continue;
        }

        std::string release_name = release.path().filename();

        if (latest_installed_release.empty()
          || latest_installed_release < release_name) {
            latest_installed_release = release_name;
        }
    }

    if (latest_installed_release.empty()) {
        std::cerr << "No installed kernels found in " << modules_path << '\n';
        return sbar::error_str;
    }

    if (running_release != latest_installed_release) {
        return "🔴";
    }

    return "🟢";
}

} // namespace sbar
