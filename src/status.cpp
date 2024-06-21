// Standard includes
#include <array>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>

// External includes
#include <sys/sysinfo.h>

// Local includes
#include "constants.hpp"
#include "proc_stat.hpp"
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

std::string time() {
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

std::string uptime() {
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

std::string disk_percent() {
    std::filesystem::space_info root_dir =
      std::filesystem::space(std::filesystem::current_path().root_path());

    auto total = static_cast<double>(root_dir.capacity);
    auto used = static_cast<double>(root_dir.capacity - root_dir.available);

    return sprintf("%.0f", (used / total) * 1e2);
}

std::string swap_percent() {
    struct sysinfo system_info {};
    if (sysinfo(&system_info) != 0) {
        return error_str;
    }

    auto total = static_cast<double>(system_info.totalswap);
    auto used =
      static_cast<double>(system_info.totalswap - system_info.freeswap);

    return sprintf("%.0f", (used / total) * 1e2);
}

std::string memory_percent() {
    struct sysinfo system_info {};
    if (sysinfo(&system_info) != 0) {
        return error_str;
    }

    auto total = static_cast<double>(system_info.totalram);
    auto used = static_cast<double>(system_info.totalram - system_info.freeram
      - system_info.bufferram - system_info.sharedram);

    return sprintf("%.0f", (used / total) * 1e2);
}

std::string cpu_percent(std::unique_ptr<cpu>& cpu_stat) {
    if (cpu_stat == nullptr) {
        cpu_stat = std::make_unique<cpu>();
        return standby_str;
    }

    try {
        std::vector<cpu::index> idle_components{ cpu::index::idle };

        auto prev_total = cpu_stat->get_total();
        auto prev_idle = cpu_stat->get_total(idle_components);
        auto prev_work = prev_total - prev_idle;

        cpu_stat = std::make_unique<cpu>();

        auto new_total = cpu_stat->get_total();
        auto new_idle = cpu_stat->get_total(idle_components);
        auto new_work = new_total - new_idle;

        auto total_diff = static_cast<double>(new_total - prev_total);
        auto work_diff = static_cast<double>(new_work - prev_work);

        return sprintf("%.0f", (work_diff / total_diff) * 1e2);
    } catch (const invalid_proc_stat& err) {
        std::cerr << err.what() << '\n';
        return err.status();
    }
}

std::string cpu_temperature() {
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

std::string one_minute_load_average() {
    std::array<double, 1> load_averages{};

    if (getloadavg(load_averages.data(), load_averages.size()) == -1) {
        return status_bar::error_str;
    }

    return sprintf("%.1f", load_averages.back());
}

std::string five_minute_load_average() {
    std::array<double, 2> load_averages{};

    if (getloadavg(load_averages.data(), load_averages.size()) == -1) {
        return status_bar::error_str;
    }

    return sprintf("%.1f", load_averages.back());
}

std::string fifteen_minute_load_average() {
    std::array<double, 3> load_averages{};

    if (getloadavg(load_averages.data(), load_averages.size()) == -1) {
        return status_bar::error_str;
    }

    return sprintf("%.1f", load_averages.back());
}

std::string battery_state() {
    const char* const devices_path = "/sys/class/power_supply/";
    const char* const device_type_filename = "type";
    const char* const device_type_battery = "Battery";
    const char* const device_status_filename = "status";
    const char* const device_status_charging = "Charging";
    const char* const device_status_discharging = "Discharging";
    const char* const device_status_not_charging = "Not Charging";
    const char* const device_status_full = "Full";

    for (const std::filesystem::directory_entry& device :
      std::filesystem::directory_iterator(devices_path)) {
        if (! device.is_directory()) {
            continue;
        }

        std::string type = get_first_line(device.path() / device_type_filename);
        if (type != device_type_battery) {
            continue;
        }

        std::string status =
          get_first_line(device.path() / device_status_filename);
        if (status == device_status_full) {
            return "🟢";
        }
        if (status == device_status_charging) {
            return "🔵";
        }
        if (status == device_status_discharging) {
            return "🟡";
        }
        if (status == device_status_not_charging) {
            return "🔴";
        }

        return status_bar::error_str;
    }

    return status_bar::error_str;
}

std::string battery_percent() {
    return status_bar::null_str;
}

std::string backlight_percent() {
    return status_bar::null_str;
}

std::string network_ssid() {
    return status_bar::null_str;
}

std::string wifi_percent() {
    return status_bar::null_str;
}

std::string bluetooth_devices() {
    return status_bar::null_str;
}

std::string volume_status() {
    return status_bar::null_str;
}

std::string volume_perc() {
    return status_bar::null_str;
}

std::string microphone_state() {
    return status_bar::null_str;
}

std::string camera_state() {
    return status_bar::null_str;
}

} // namespace status_bar
