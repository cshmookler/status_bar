// Standard includes
#include <array>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
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
#include <alsa/asoundlib.h>
#include <linux/videodev2.h>
#include <linux/wireless.h>
#include <pwd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <unistd.h>

// Local includes
#include "constants.hpp"
#include "status.hpp"

namespace sbar {

template<typename... Args>
[[nodiscard]] std::string sprintf(const char* format, Args... args) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, hicpp-vararg)
    int size = std::snprintf(nullptr, 0, format, args...);
    std::string buffer(size, '\0');
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, hicpp-vararg)
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
        return sbar::null_str;
    }
    return first_line;
}

[[nodiscard]] std::string_view split(std::string_view& str, char delimiter) {
    size_t delimiter_index = 0;
    std::string_view return_value;
    while (delimiter_index == 0) {
        delimiter_index = str.find(delimiter);
        if (delimiter_index == std::string::npos) {
            return sbar::null_str;
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
        if (split_str == sbar::null_str) {
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

        this->entries_ = split<index_count>(line_c_str, ' ');

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

std::string get_one_minute_load_average() {
    std::array<double, 1> load_averages{};

    if (getloadavg(load_averages.data(), load_averages.size()) == -1) {
        return sbar::error_str;
    }

    return sprintf("%.1f", load_averages.back());
}

std::string get_five_minute_load_average() {
    std::array<double, 2> load_averages{};

    if (getloadavg(load_averages.data(), load_averages.size()) == -1) {
        return sbar::error_str;
    }

    return sprintf("%.1f", load_averages.back());
}

std::string get_fifteen_minute_load_average() {
    std::array<double, 3> load_averages{};

    if (getloadavg(load_averages.data(), load_averages.size()) == -1) {
        return sbar::error_str;
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
    const char* const battery_status_not_charging = "Not charging";
    const char* const battery_status_full = "Full";

    const int medium_battery_percent = 60;
    const int low_battery_percent = 40;
    const int very_low_battery_percent = 20;

    std::string status = get_first_line(battery_path / battery_status_filename);

    if (status == battery_status_full || status == battery_status_charging) {
        return "🟢";
    }
    if (status == battery_status_not_charging) {
        return "❌";
    }

    if (status != battery_status_discharging) {
        return sbar::error_str;
    }

    std::string battery_percent = get_battery_percent(battery_path);
    if (battery_percent == sbar::error_str) {
        return sbar::error_str;
    }

    int battery_percent_numeric = std::stoi(battery_percent);
    if (battery_percent_numeric <= very_low_battery_percent) {
        return "🔴";
    }
    if (battery_percent_numeric <= low_battery_percent) {
        return "🟠";
    }
    if (battery_percent_numeric <= medium_battery_percent) {
        return "🟡";
    }
    return "🔵";
}

std::string get_battery_device(const std::filesystem::path& battery_path) {
    return battery_path.stem();
}

std::string get_battery_percent(const std::filesystem::path& battery_path) {
    // documentation for /sys/class/power_supply/:
    // https://github.com/torvalds/linux/blob/master/include/linux/power_supply.h
    // https://www.kernel.org/doc/html/latest/power/power_supply_class.html

    const char* const battery_capacity_filename = "capacity";

    std::string capacity =
      get_first_line(battery_path / battery_capacity_filename);
    if (capacity == sbar::null_str) {
        return sbar::error_str;
    }

    return capacity;
}

bool Battery_state::add_sample(const std::filesystem::path& battery_path) {
    // documentation for /sys/class/power_supply/:
    // https://github.com/torvalds/linux/blob/master/include/linux/power_supply.h
    // https://www.kernel.org/doc/html/latest/power/power_supply_class.html

    const char* const battery_energy_now_filename = "energy_now";

    std::string energy_now =
      get_first_line(battery_path / battery_energy_now_filename);
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

std::string get_battery_time_remaining(
  const std::filesystem::path& battery_path,
  Battery_state& battery_state_info) {
    if (! battery_state_info.add_sample(battery_path)) {
        return sbar::error_str;
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
        return sbar::error_str;
    }

    std::string brightness =
      get_first_line(backlight_path.value() / battery_brightness_filename);
    if (brightness == sbar::null_str) {
        return sbar::error_str;
    }

    std::string max_brightness =
      get_first_line(backlight_path.value() / battery_max_brightness_filename);
    if (max_brightness == sbar::null_str) {
        return sbar::error_str;
    }

    return sprintf(
      "%.0f", std::stof(brightness) / std::stof(max_brightness) * 1e2);
}

std::optional<std::filesystem::path> get_network() {
    // documentation for /sys/class/net/:
    // https://github.com/torvalds/linux/blob/master/include/linux/net.h
    // https://www.kernel.org/doc/html/latest/driver-api/input.html

    const char* const networks_path = "/sys/class/net/";
    const char* const network_operstate_filename = "operstate";
    const char* const network_device_path = "device/";
    const char* const network_statistics_path = "statistics/";
    const char* const network_statistics_rx_bytes_filename = "rx_bytes";
    const char* const network_statistics_tx_bytes_filename = "tx_bytes";

    for (const std::filesystem::directory_entry& device :
      std::filesystem::directory_iterator(networks_path)) {
        if (! std::filesystem::exists(
              device.path() / network_operstate_filename)) {
            continue;
        }
        if (! std::filesystem::exists(device.path() / network_device_path)) {
            continue;
        }
        if (! std::filesystem::exists(device.path() / network_statistics_path
              / network_statistics_rx_bytes_filename)) {
            continue;
        }
        if (! std::filesystem::exists(device.path() / network_statistics_path
              / network_statistics_tx_bytes_filename)) {
            continue;
        }

        return device.path();
    }

    return std::nullopt;
}

std::string get_network_status(
  const std::filesystem::path& network_interface_path) {
    // documentation for /sys/class/net/:
    // https://github.com/torvalds/linux/blob/master/include/linux/net.h
    // https://www.kernel.org/doc/html/latest/driver-api/input.html

    const char* const network_operstate_filename = "operstate";
    // const char* const network_operstate_unknown = "unknown";
    const char* const network_operstate_up = "up";
    const char* const network_operstate_dormant = "dormant";
    const char* const network_operstate_down = "down";

    std::string operstate =
      get_first_line(network_interface_path / network_operstate_filename);
    if (operstate == sbar::null_str) {
        return sbar::error_str;
    }

    if (operstate == network_operstate_up) {
        return "🟢";
    }
    if (operstate == network_operstate_dormant) {
        return "🟡";
    }
    if (operstate == network_operstate_down) {
        return "🔴";
    }

    return sbar::error_str;
}

std::string get_network_device(
  const std::filesystem::path& network_interface_path) {
    return network_interface_path.stem();
}

template<typename... Request_t>
bool request(
  int file_descriptor, unsigned long request_type, Request_t&... request) {
    auto result = ioctl(file_descriptor, request_type, &request...);
    int err = errno;
    if (result < 0) {
        std::cerr << "ioctl(" << request_type << "): " << std::strerror(err)
                  << '\n';
    }
    return result >= 0;
}

class Unix_socket {
    static const int default_protocol = 0;

    int socket_file_descriptor_;

  public:
    Unix_socket(int domain, int type, int protocol = default_protocol)
    : socket_file_descriptor_(socket(domain, type, protocol)) {
    }

    Unix_socket(const Unix_socket&) = delete;
    Unix_socket(Unix_socket&&) noexcept = default;
    Unix_socket& operator=(const Unix_socket&) = delete;
    Unix_socket& operator=(Unix_socket&&) noexcept = default;

    ~Unix_socket() {
        close(this->socket_file_descriptor_);
        // do nothing if the socket fails to close.
    }

    [[nodiscard]] bool good() const {
        return this->socket_file_descriptor_ >= 0;
    }

    template<typename... Request_t>
    bool request(unsigned long request_type, Request_t&... request) {
        return ::sbar::request(
          this->socket_file_descriptor_, request_type, request...);
    }
};

std::string get_network_ssid(
  const std::filesystem::path& network_interface_path) {
    // documentation:
    // https://github.com/torvalds/linux/blob/master/include/uapi/linux/wireless.h

    Unix_socket socket{ AF_INET, SOCK_DGRAM };
    if (! socket.good()) {
        return sbar::error_str;
    }

    iwreq iwreq_info{};

    std::strcpy(iwreq_info.ifr_ifrn.ifrn_name,
      network_interface_path.stem().string().data());

    // This array must be 1 unit larger than the maximum ESSID size and default
    // initialized so that the ESSID is null-terminated.
    std::array<char, IW_ESSID_MAX_SIZE + 1> essid{};

    iwreq_info.u.essid.pointer = essid.data();
    iwreq_info.u.essid.length = essid.size();

    if (! socket.request(SIOCGIWESSID, iwreq_info)) {
        return sbar::error_str;
    }

    return std::string{ essid.data() };
}

std::string get_network_signal_strength_percent(
  const std::filesystem::path& network_interface_path) {
    // documentation:
    // https://github.com/torvalds/linux/blob/master/include/uapi/linux/wireless.h

    Unix_socket socket{ AF_INET, SOCK_DGRAM };
    if (! socket.good()) {
        return sbar::error_str;
    }

    iwreq iwreq_info{};
    iw_statistics iw_statistics_info{};
    iwreq_info.u.data.pointer = &iw_statistics_info;
    iwreq_info.u.data.length = sizeof(iw_statistics_info);

    std::strcpy(iwreq_info.ifr_ifrn.ifrn_name,
      network_interface_path.stem().string().data());

    if (! socket.request(SIOCGIWSTATS, iwreq_info)) {
        return sbar::error_str;
    }

    const double max_signal_strength = 70;
    double signal_strength =
      iw_statistics_info.qual.qual / max_signal_strength * 1e2;

    return sprintf("%.0f", signal_strength);
}

size_t Network_state::get_upload_byte_difference(size_t upload_byte_count) {
    size_t difference = upload_byte_count - this->upload_byte_count_;
    this->upload_byte_count_ = upload_byte_count;
    return difference;
}

size_t Network_state::get_download_byte_difference(size_t download_byte_count) {
    size_t difference = download_byte_count - this->download_byte_count_;
    this->download_byte_count_ = download_byte_count;
    return difference;
}

std::string get_network_upload(
  const std::filesystem::path& network_interface_path,
  Network_state& network_state_info) {
    // documentation for /sys/class/net/:
    // https://github.com/torvalds/linux/blob/master/include/linux/net.h
    // https://www.kernel.org/doc/html/latest/driver-api/input.html

    const char* const network_statistics_path = "statistics/";
    const char* const network_statistics_tx_bytes_filename = "tx_bytes";

    std::string upload_bytes = get_first_line(network_interface_path
      / network_statistics_path / network_statistics_tx_bytes_filename);
    if (upload_bytes == sbar::null_str) {
        return sbar::error_str;
    }

    size_t upload_bytes_numeric = std::stoull(upload_bytes);

    auto upload_byte_difference =
      network_state_info.get_upload_byte_difference(upload_bytes_numeric);
    if (upload_byte_difference == upload_bytes_numeric) {
        return sbar::standby_str;
    }

    return sprintf("%i", upload_byte_difference);
}

std::string get_network_download(
  const std::filesystem::path& network_interface_path,
  Network_state& network_state_info) {
    // documentation for /sys/class/net/:
    // https://github.com/torvalds/linux/blob/master/include/linux/net.h
    // https://www.kernel.org/doc/html/latest/driver-api/input.html

    const char* const network_statistics_path = "statistics/";
    const char* const network_statistics_rx_bytes_filename = "rx_bytes";

    std::string download_bytes = get_first_line(network_interface_path
      / network_statistics_path / network_statistics_rx_bytes_filename);
    if (download_bytes == sbar::null_str) {
        return sbar::error_str;
    }

    size_t download_bytes_numeric = std::stoull(download_bytes);

    auto download_byte_difference =
      network_state_info.get_download_byte_difference(download_bytes_numeric);
    if (download_byte_difference == download_bytes_numeric) {
        return sbar::standby_str;
    }

    return sprintf("%i", download_byte_difference);
}

class Sound_mixer {
    static constexpr const char* default_card = "default";

    static constexpr const char* playback_name = "Master";
    static constexpr int playback_index = 0;

    static constexpr const char* capture_name = "Capture";
    static constexpr int capture_index = 0;

    static constexpr int mixer_mode = 0;

    snd_mixer_t* mixer_ = nullptr;

    bool good_ = false;

    template<typename F, typename... A>
    [[nodiscard]] static bool handle_error_(
      const std::string_view& function_name, F function, A... args) {
        static_assert(std::is_invocable_v<F, A...>);

        int errnum = function(args...);
        if (errnum == 0) {
            return false;
        }

        std::cerr << function_name << "(): " << snd_strerror(errnum) << '\n';

        return true;
    }

    [[nodiscard]] static const char* get_indicator_(int state) {
        if (state == 0) {
            return "🔴";
        }
        return "🟢";
    }

    [[nodiscard]] static long get_percent_(long min, long max, long value) {
        double ratio =
          static_cast<double>(value - min) / static_cast<double>(max);
        return static_cast<long>(ratio * 100.F);
    }

    template<typename F>
    [[nodiscard]] static std::string get_state_(
      const char* switch_function_name,
      F switch_function,
      snd_mixer_elem_t* mixer_elem) {
        int left_state = 0;
        if (handle_error_(switch_function_name,
              switch_function,
              mixer_elem,
              snd_mixer_selem_channel_id_t::SND_MIXER_SCHN_FRONT_LEFT,
              &left_state)) {
            return sbar::error_str;
        }

        int right_state = 0;
        if (handle_error_(switch_function_name,
              switch_function,
              mixer_elem,
              snd_mixer_selem_channel_id_t::SND_MIXER_SCHN_FRONT_RIGHT,
              &right_state)) {
            return sbar::error_str;
        }

        if (left_state != right_state) {
            return sprintf("(%s, %s)",
              get_indicator_(left_state),
              get_indicator_(right_state));
        }

        return get_indicator_(left_state);
    }

    template<typename R, typename V>
    [[nodiscard]] static std::string get_volume_(
      const char* volume_range_function_name,
      R volume_range_function,
      const char* volume_function_name,
      V volume_function,
      snd_mixer_elem_t* mixer_elem) {
        if (mixer_elem == nullptr) {
            return sbar::error_str;
        }

        long min_volume = -1;
        long max_volume = -1;
        if (handle_error_(volume_range_function_name,
              volume_range_function,
              mixer_elem,
              &min_volume,
              &max_volume)) {
            return sbar::error_str;
        }

        long left_volume = -1;
        if (handle_error_(volume_function_name,
              volume_function,
              mixer_elem,
              snd_mixer_selem_channel_id_t::SND_MIXER_SCHN_FRONT_LEFT,
              &left_volume)) {
            return sbar::error_str;
        }

        long right_volume = -1;
        if (handle_error_(volume_function_name,
              volume_function,
              mixer_elem,
              snd_mixer_selem_channel_id_t::SND_MIXER_SCHN_FRONT_RIGHT,
              &right_volume)) {
            return sbar::error_str;
        }

        long left_volume_percent =
          get_percent_(min_volume, max_volume, left_volume);

        long right_volume_percent =
          get_percent_(min_volume, max_volume, right_volume);

        if (left_volume != right_volume) {
            return sprintf(
              "(%i, %i)", left_volume_percent, right_volume_percent);
        }

        return sprintf("%i", left_volume_percent);
    }

    [[nodiscard]] snd_mixer_elem_t* get_mixer_elem_(
      const char* selem_name, int selem_index) const {
        if (! this->good()) {
            return nullptr;
        }

        snd_mixer_selem_id_t* selem = nullptr;
        if (handle_error_(
              "snd_mixer_selem_id_malloc", snd_mixer_selem_id_malloc, &selem)) {
            return nullptr;
        }

        snd_mixer_selem_id_set_name(selem, selem_name);
        snd_mixer_selem_id_set_index(selem, selem_index);

        return snd_mixer_find_selem(this->mixer_, selem);
    }

  public:
    Sound_mixer() {
        if (handle_error_(
              "snd_mixer_open", snd_mixer_open, &this->mixer_, mixer_mode)) {
            return;
        }
        if (handle_error_("snd_mixer_attach",
              snd_mixer_attach,
              this->mixer_,
              default_card)) {
            return;
        }
        if (handle_error_("snd_mixer_selem_register",
              snd_mixer_selem_register,
              this->mixer_,
              nullptr,
              nullptr)) {
            return;
        }
        if (handle_error_("snd_mixer_load", snd_mixer_load, this->mixer_)) {
            return;
        }
        this->good_ = true;
    }

    Sound_mixer(const Sound_mixer&) = delete;
    Sound_mixer(Sound_mixer&&) noexcept = default;
    Sound_mixer& operator=(const Sound_mixer&) = delete;
    Sound_mixer& operator=(Sound_mixer&&) noexcept = default;

    ~Sound_mixer() {
        snd_mixer_close(this->mixer_);
        // do nothing if the mixer fails to close.
    }

    [[nodiscard]] bool good() const {
        return this->good_;
    }

    [[nodiscard]] std::string get_playback_state() const {
        return get_state_("snd_mixer_selem_get_playback_switch",
          snd_mixer_selem_get_playback_switch,
          this->get_mixer_elem_(playback_name, playback_index));
    }

    [[nodiscard]] std::string get_playback_volume() const {
        return get_volume_("snd_mixer_selem_get_playback_volume_range",
          snd_mixer_selem_get_playback_volume_range,
          "snd_mixer_selem_get_playback_volume",
          snd_mixer_selem_get_playback_volume,

          this->get_mixer_elem_(playback_name, playback_index));
    }

    [[nodiscard]] std::string get_capture_state() const {
        return get_state_("snd_mixer_selem_get_capture_switch",
          snd_mixer_selem_get_capture_switch,
          this->get_mixer_elem_(capture_name, capture_index));
    }

    [[nodiscard]] std::string get_capture_volume() const {
        return get_volume_("snd_mixer_selem_get_capture_volume_range",
          snd_mixer_selem_get_capture_volume_range,
          "snd_mixer_selem_get_capture_volume",
          snd_mixer_selem_get_capture_volume,
          this->get_mixer_elem_(capture_name, capture_index));
    }
};

std::string get_volume_state() {
    Sound_mixer mixer{};
    return mixer.get_playback_state();
}

std::string get_volume_perc() {
    Sound_mixer mixer{};
    return mixer.get_playback_volume();
}

std::string get_capture_state() {
    Sound_mixer mixer{};
    return mixer.get_capture_state();
}

std::string get_capture_perc() {
    Sound_mixer mixer{};
    return mixer.get_capture_volume();
}

std::string get_microphone_state() {
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

std::string get_camera_state() {
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
