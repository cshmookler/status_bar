// Standard includes
#include <cstddef>
#include <fstream>
#include <string>
#include <string_view>

// Local includes
#include "../status.hpp"

namespace sbar {

bool Cpu::update_stat() {
    const char* const proc_stat_path = "/proc/stat";
    const char* const proc_stat_cpu_field = "cpu ";

    std::string first_line;
    {
        std::ifstream proc_stat{ proc_stat_path };
        first_line = get_first_line(proc_stat_path) + ' ';
    }

    auto first_line_c_str =
      std::string_view{ first_line.data(), first_line.size() };

    if (! remove_prefix(first_line_c_str, proc_stat_cpu_field)) {
        return false;
    }

    for (size_t& entry : this->stat_) {
        std::string entry_str{ split(first_line_c_str, ' ') };
        try {
            entry = std::stoull(entry_str);
        } catch (const std::invalid_argument& error) {
            return false;
        }
    }

    this->ready_ = true;
    return true;
}

size_t Cpu::get_total() const {
    size_t total = 0;
    for (size_t entry : this->stat_) {
        total += entry;
    }
    return total;
}

size_t Cpu::get_total(cpu_stat stat) const {
    size_t total = 0;
    for (size_t i = 0; i < cpu_stat_count; i++) {
        if ((stat & bit(i)) != cpu_stat_none) {
            total += this->stat_.at(i);
        }
    }
    return total;
}

std::string Fields::get_cpu_percent() {
    if (! this->cpu.ready()) {
        if (! this->cpu.update_stat()) {
            return sbar::error_str;
        }
        return sbar::standby_str;
    }

    cpu_stat idle_components = cpu_stat::idle;

    auto prev_total = this->cpu.get_total();
    auto prev_idle = this->cpu.get_total(idle_components);
    auto prev_work = prev_total - prev_idle;

    if (! this->cpu.update_stat()) {
        return sbar::error_str;
    }

    auto new_total = this->cpu.get_total();
    auto new_idle = this->cpu.get_total(idle_components);
    auto new_work = new_total - new_idle;

    auto total_diff = static_cast<double>(new_total - prev_total);
    auto work_diff = static_cast<double>(new_work - prev_work);

    return sprintf("%2.0f", (work_diff / total_diff) * 1e2);
}

std::string Fields::get_cpu_temperature() {
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

} // namespace sbar
