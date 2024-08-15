// Local includes
#include "../status.hpp"

namespace sbar {

bool Battery::init() {
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

        this->path_ = device.path();
        this->good_ = true;
        return true;
    }

    this->good_ = false;
    return false;
}

bool Battery::add_sample() {
    // documentation for /sys/class/power_supply/:
    // https://github.com/torvalds/linux/blob/master/include/linux/power_supply.h
    // https://www.kernel.org/doc/html/latest/power/power_supply_class.html

    const char* const battery_energy_now_filename = "energy_now";

    std::string energy_now =
      get_first_line(this->path() / battery_energy_now_filename);
    if (energy_now == sbar::null_str) {
        return false;
    }

    if (this->has_enough_samples()) {
        this->energy_remaining_.pop_front();
    }

    this->energy_remaining_.push_back(std::stoull(energy_now));

    return true;
}

bool Battery::has_enough_samples() const {
    return this->energy_remaining_.size() >= sample_size;
}

std::string Battery::get_time_remaining() const {
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

std::string get_battery_status(Battery& battery) {
    if (! battery.good() && ! battery.init()) {
        return sbar::error_str;
    }

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

    std::string status =
      get_first_line(battery.path() / battery_status_filename);

    if (status == battery_status_full || status == battery_status_charging) {
        return "🟢";
    }
    if (status == battery_status_not_charging) {
        return "❌";
    }

    if (status != battery_status_discharging) {
        return sbar::error_str;
    }

    std::string battery_percent = get_battery_percent(battery);
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

std::string get_battery_device(Battery& battery) {
    if (! battery.good() && ! battery.init()) {
        return sbar::error_str;
    }
    return battery->stem();
}

std::string get_battery_percent(Battery& battery) {
    if (! battery.good() && ! battery.init()) {
        return sbar::error_str;
    }

    // documentation for /sys/class/power_supply/:
    // https://github.com/torvalds/linux/blob/master/include/linux/power_supply.h
    // https://www.kernel.org/doc/html/latest/power/power_supply_class.html

    const char* const battery_capacity_filename = "capacity";

    std::string capacity =
      get_first_line(battery.path() / battery_capacity_filename);
    if (capacity == sbar::null_str) {
        return sbar::error_str;
    }

    return capacity;
}

std::string get_battery_time_remaining(Battery& battery) {
    if (! battery.good() && ! battery.init() && ! battery.add_sample()) {
        return sbar::error_str;
    }
    return battery.get_time_remaining();
}

} // namespace sbar
