// Local includes
#include "../status.hpp"

namespace sbar {

bool Backlight::init() {
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
        return true;
    }

    this->good_ = false;
    return false;
}

std::string get_backlight_percent(Backlight& backlight) {
    if (! backlight.good() && ! backlight.init()) {
        return sbar::error_str;
    }

    // documentation for /sys/class/backlight/:
    // https://github.com/torvalds/linux/blob/master/include/linux/backlight.h
    // https://docs.kernel.org/gpu/backlight.html

    const char* const device_brightness_filename = "brightness";
    const char* const device_max_brightness_filename = "max_brightness";

    std::string brightness =
      get_first_line(backlight.path() / device_brightness_filename);
    if (brightness == sbar::null_str) {
        return sbar::error_str;
    }

    std::string max_brightness =
      get_first_line(backlight.path() / device_max_brightness_filename);
    if (max_brightness == sbar::null_str) {
        return sbar::error_str;
    }

    return sprintf(
      "%.0f", std::stof(brightness) / std::stof(max_brightness) * 1e2);
}

} // namespace sbar
