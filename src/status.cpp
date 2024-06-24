// Standard includes
#include <array>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// External includes
#include <sys/sysinfo.h>

// Local includes
#include "constants.hpp"
#include "status.hpp"

namespace status_bar {

template<typename... Args>
[[nodiscard]] std::string sprintf(const char* format, Args... args) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg,
    // hicpp-vararg)
    int size = std::snprintf(nullptr, 0, format, args...);
    std::string buffer(size, '\0');
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg,
    // hicpp-vararg)
    if (std::sprintf(buffer.data(), format, args...) < 0) {
        return error_str;
    }
    return buffer;
}

[[nodiscard]] bool remove_prefix(
  std::string_view& target, const std::string_view& prefix) {
    if (prefix.size() > target.size()) {
        return false;
    }

    bool found_prefix = target.rfind(prefix, 0) == 0;

    if (found_prefix) {
        target = target.substr(prefix.size());
    }

    return found_prefix;
}

[[nodiscard]] bool remove_postfix(
  std::string_view& target, const std::string_view& postfix) {
    if (postfix.size() > target.size()) {
        return false;
    }

    size_t postfix_pos = target.size() - postfix.size();
    bool found_postfix = target.find(postfix, postfix_pos) == postfix_pos;

    if (found_postfix) {
        target = target.substr(0, target.size() - postfix.size());
    }

    return found_postfix;
}

[[nodiscard]] std::string get_first_line(const std::filesystem::path& path) {
    std::ifstream file{ path };
    std::string first_line;
    if (! std::getline(file, first_line).good()) {
        return status_bar::null_str;
    }
    return first_line;
}

[[nodiscard]] std::string_view split(std::string_view& str, char delimiter) {
    size_t delimiter_index = 0;
    std::string_view return_value;
    while (delimiter_index == 0) {
        delimiter_index = str.find(delimiter);
        if (delimiter_index == std::string::npos) {
            return status_bar::null_str;
        }
        return_value = str.substr(0, delimiter_index);
        str = str.substr(delimiter_index + 1, str.size() - delimiter_index + 1);
    }
    return return_value;
}

template<size_t count>
[[nodiscard]] std::array<size_t, count> split(
  std::string_view str, char delimiter) {
    std::array<size_t, count> fields{};
    for (size_t& field : fields) {
        std::string split_str{ split(str, delimiter) };
        if (split_str == status_bar::null_str) {
            field = 0;
        } else {
            field = std::stoull(split_str);
        }
    }
    return fields;
}

std::string get_time() {
    std::time_t epoch_time = std::time(nullptr);
    std::tm* calendar_time = std::localtime(&epoch_time);

    // RFC 3339 format
    return sprintf("%i-%.2i-%.2i %.2i:%.2i:%.2i",
      calendar_time->tm_year + 1900, // NOLINT(readability-magic-numbers)
      calendar_time->tm_mon + 1,
      calendar_time->tm_mday,
      calendar_time->tm_hour,
      calendar_time->tm_min,
      calendar_time->tm_sec);
}

std::string get_uptime() {
    struct sysinfo system_info {};
    if (sysinfo(&system_info) != 0) {
        return error_str;
    }

    std::time_t epoch_uptime = system_info.uptime;
    std::tm* calendar_uptime = std::gmtime(&epoch_uptime);

    // non-standard format
    return sprintf("%i-%.3i %.2i:%.2i:%.2i",
      calendar_uptime->tm_year - 70, // NOLINT(readability-magic-numbers)
      calendar_uptime->tm_yday,
      calendar_uptime->tm_hour,
      calendar_uptime->tm_min,
      calendar_uptime->tm_sec);
}

std::string get_disk_percent() {
    std::filesystem::space_info root_dir =
      std::filesystem::space(std::filesystem::current_path().root_path());

    auto total = static_cast<double>(root_dir.capacity);
    auto used = static_cast<double>(root_dir.capacity - root_dir.available);

    return sprintf("%.0f", (used / total) * 1e2);
}

std::string get_swap_percent() {
    struct sysinfo system_info {};
    if (sysinfo(&system_info) != 0) {
        return error_str;
    }

    auto total = static_cast<double>(system_info.totalswap);
    auto used =
      static_cast<double>(system_info.totalswap - system_info.freeswap);

    return sprintf("%.0f", (used / total) * 1e2);
}

std::string get_memory_percent() {
    struct sysinfo system_info {};
    if (sysinfo(&system_info) != 0) {
        return error_str;
    }

    auto total = static_cast<double>(system_info.totalram);
    auto used = static_cast<double>(system_info.totalram - system_info.freeram
      - system_info.bufferram - system_info.sharedram);

    return sprintf("%.0f", (used / total) * 1e2);
}

bool cpu_state::update() {
    const char* const proc_stat_path = "/proc/stat";
    const char* const proc_stat_cpu_field = "cpu ";

    std::ifstream proc_stat{ proc_stat_path };

    std::string line;
    while (std::getline(proc_stat, line).good()) {
        auto line_c_str = std::string_view{ line.data(), line.size() };
        if (! remove_prefix(line_c_str, proc_stat_cpu_field)) {
            continue;
        }

        this->entries_ = split<index_count>(line_c_str, ' ');

        return true;
    }

    return false;
}

size_t cpu_state::get_total() const {
    size_t total = 0;
    for (size_t entry : this->entries_) {
        total += entry;
    }
    return total;
}

size_t cpu_state::get_total(const std::vector<index>& indicies) const {
    size_t total = 0;
    for (index idx : indicies) {
        total += this->entries_.at(static_cast<size_t>(idx));
    }
    return total;
}

std::string get_cpu_percent(std::unique_ptr<cpu_state>& cpu_state_info) {
    if (cpu_state_info == nullptr) {
        cpu_state_info = std::make_unique<cpu_state>();
        if (! cpu_state_info->update()) {
            return status_bar::error_str;
        }
        return status_bar::standby_str;
    }

    std::vector<cpu_state::index> idle_components{ cpu_state::index::idle };

    auto prev_total = cpu_state_info->get_total();
    auto prev_idle = cpu_state_info->get_total(idle_components);
    auto prev_work = prev_total - prev_idle;

    if (! cpu_state_info->update()) {
        return status_bar::error_str;
    }

    auto new_total = cpu_state_info->get_total();
    auto new_idle = cpu_state_info->get_total(idle_components);
    auto new_work = new_total - new_idle;

    auto total_diff = static_cast<double>(new_total - prev_total);
    auto work_diff = static_cast<double>(new_work - prev_work);

    return sprintf("%.0f", (work_diff / total_diff) * 1e2);
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
            if (sensor_input == status_bar::null_str) {
                continue;
            }

            return sprintf(
              "%.0f", static_cast<float>(std::stoull(sensor_input)) / 1e3F);
        }
    }

    return status_bar::error_str;
}

std::string get_one_minute_load_average() {
    std::array<double, 1> load_averages{};

    if (getloadavg(load_averages.data(), load_averages.size()) == -1) {
        return status_bar::error_str;
    }

    return sprintf("%.1f", load_averages.back());
}

std::string get_five_minute_load_average() {
    std::array<double, 2> load_averages{};

    if (getloadavg(load_averages.data(), load_averages.size()) == -1) {
        return status_bar::error_str;
    }

    return sprintf("%.1f", load_averages.back());
}

std::string get_fifteen_minute_load_average() {
    std::array<double, 3> load_averages{};

    if (getloadavg(load_averages.data(), load_averages.size()) == -1) {
        return status_bar::error_str;
    }

    return sprintf("%.1f", load_averages.back());
}

std::optional<std::filesystem::path> get_battery() {
    // documentation for /sys/class/power_supply/:
    // https://github.com/torvalds/linux/blob/master/include/linux/power_supply.h
    // https://www.kernel.org/doc/html/latest/power/power_supply_class.html

    const char* const devices_path = "/sys/class/power_supply/";
    const char* const device_type_filename = "type";
    const char* const device_type_battery = "Battery";
    const char* const device_status_filename = "status";
    const char* const device_capacity_filename = "capacity";
    const char* const device_energy_now_filename = "energy_now";

    for (const std::filesystem::directory_entry& device :
      std::filesystem::directory_iterator(devices_path)) {
        std::string type = get_first_line(device.path() / device_type_filename);
        if (type != device_type_battery) {
            continue;
        }

        if (! std::filesystem::exists(device.path() / device_status_filename)) {
            continue;
        }
        if (! std::filesystem::exists(
              device.path() / device_capacity_filename)) {
            continue;
        }
        if (! std::filesystem::exists(
              device.path() / device_energy_now_filename)) {
            continue;
        }

        return device.path();
    }

    return std::nullopt;
}

std::string get_battery_status(const std::filesystem::path& battery_path) {
    // documentation for /sys/class/power_supply/:
    // https://github.com/torvalds/linux/blob/master/include/linux/power_supply.h
    // https://www.kernel.org/doc/html/latest/power/power_supply_class.html

    const char* const battery_status_filename = "status";
    // const char* const battery_status_unknown = "Unknown";
    const char* const battery_status_charging = "Charging";
    const char* const battery_status_discharging = "Discharging";
    const char* const battery_status_not_charging = "Not Charging";
    const char* const battery_status_full = "Full";

    const int low_battery_percent = 20;

    std::string status = get_first_line(battery_path / battery_status_filename);
    if (status == battery_status_full) {
        return "🟢";
    }
    if (status == battery_status_charging) {
        return "🔵";
    }
    if (status == battery_status_not_charging) {
        return "❌";
    }
    if (status == battery_status_discharging) {
        std::string battery_percent = get_battery_percent(battery_path);
        if (battery_percent == status_bar::error_str) {
            return status_bar::error_str;
        }
        if (std::stoi(battery_percent) <= low_battery_percent) {
            return "🔴";
        }

        return "🟡";
    }

    return status_bar::error_str;
}

std::string get_battery_percent(const std::filesystem::path& battery_path) {
    // documentation for /sys/class/power_supply/:
    // https://github.com/torvalds/linux/blob/master/include/linux/power_supply.h
    // https://www.kernel.org/doc/html/latest/power/power_supply_class.html

    const char* const battery_capacity_filename = "capacity";

    std::string capacity =
      get_first_line(battery_path / battery_capacity_filename);
    if (capacity == status_bar::null_str) {
        return status_bar::error_str;
    }

    return capacity;
}

bool battery_state::add_sample(const std::filesystem::path& battery_path) {
    // documentation for /sys/class/power_supply/:
    // https://github.com/torvalds/linux/blob/master/include/linux/power_supply.h
    // https://www.kernel.org/doc/html/latest/power/power_supply_class.html

    const char* const battery_energy_now_filename = "energy_now";

    std::string energy_now =
      get_first_line(battery_path / battery_energy_now_filename);
    if (energy_now == status_bar::null_str) {
        return false;
    }

    if (this->has_enough_samples()) {
        this->energy_remaining_.pop_front();
    }

    this->energy_remaining_.push_back(std::stoull(energy_now));

    return true;
}

bool battery_state::has_enough_samples() const {
    return this->energy_remaining_.size() >= sample_size;
}

std::string battery_state::get_time_remaining() const {
    if (! this->has_enough_samples()) {
        return status_bar::standby_str;
    }

    size_t largest_sample = this->energy_remaining_.front();
    size_t smallest_sample = this->energy_remaining_.back();

    size_t difference = largest_sample - smallest_sample;
    if (difference == 0) {
        return status_bar::error_str;
    }
    size_t sample_periods_until_empty = largest_sample / difference;
    size_t seconds_until_empty = sample_periods_until_empty * sample_size;

    const size_t seconds_per_minute = 60;
    const size_t seconds_per_hour = 3600;

    size_t minutes_until_empty = seconds_until_empty % seconds_per_minute;
    size_t hours_until_empty = seconds_until_empty / seconds_per_hour;

    return sprintf("%.2i:%.2i", hours_until_empty, minutes_until_empty);
}

std::string get_battery_time_remaining(
  const std::filesystem::path& battery_path,
  battery_state& battery_state_info) {
    if (! battery_state_info.add_sample(battery_path)) {
        return status_bar::error_str;
    }

    return battery_state_info.get_time_remaining();
}

std::optional<std::filesystem::path> get_backlight() {
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

        return device.path();
    }

    return std::nullopt;
}

std::string get_backlight_percent() {
    // documentation for /sys/class/backlight/:
    // https://github.com/torvalds/linux/blob/master/include/linux/backlight.h
    // https://docs.kernel.org/gpu/backlight.html

    const char* const battery_brightness_filename = "brightness";
    const char* const battery_max_brightness_filename = "max_brightness";

    auto backlight_path = get_backlight();
    if (! backlight_path.has_value()) {
        return status_bar::error_str;
    }

    std::string brightness =
      get_first_line(backlight_path.value() / battery_brightness_filename);
    if (brightness == status_bar::null_str) {
        return status_bar::error_str;
    }

    std::string max_brightness =
      get_first_line(backlight_path.value() / battery_max_brightness_filename);
    if (max_brightness == status_bar::null_str) {
        return status_bar::error_str;
    }

    return sprintf(
      "%.0f", std::stof(brightness) / std::stof(max_brightness) * 1e2);
}

std::string get_network_ssid(
  const std::filesystem::path& network_interface_path) {
    return status_bar::null_str;
}

std::string get_wifi_percent(
  const std::filesystem::path& network_interface_path) {
    return status_bar::null_str;
}

std::string get_bluetooth_devices() {
    return status_bar::null_str;
}

std::string get_volume_status() {
    return status_bar::null_str;
}

std::string get_volume_perc() {
    return status_bar::null_str;
}

std::string get_microphone_state() {
    return status_bar::null_str;
}

std::string get_camera_state() {
    return status_bar::null_str;
}

} // namespace status_bar
