// Local includes
#include "../status.hpp"

namespace sbar {

std::optional<Battery> Optional_battery::constructor_() {
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

std::string get_battery_status(const Battery& battery) {
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

    std::string status = get_first_line(battery / battery_status_filename);

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

std::string get_battery_device(const Battery& battery) {
    return battery.stem();
}

std::string get_battery_percent(const Battery& battery) {
    // documentation for /sys/class/power_supply/:
    // https://github.com/torvalds/linux/blob/master/include/linux/power_supply.h
    // https://www.kernel.org/doc/html/latest/power/power_supply_class.html

    const char* const battery_capacity_filename = "capacity";

    std::string capacity = get_first_line(battery / battery_capacity_filename);
    if (capacity == sbar::null_str) {
        return sbar::error_str;
    }

    return capacity;
}

std::string get_battery_time_remaining(
  const Battery& battery, Battery_state& battery_state_info) {
    if (! battery_state_info.add_sample(battery)) {
        return sbar::error_str;
    }

    return battery_state_info.get_time_remaining();
}

} // namespace sbar
