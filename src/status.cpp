// Local includes
#include "status.hpp"

namespace sbar {

void Fields::reset() {
    system.reset();
    battery.reset();
    backlight.reset();
    network.reset();
    sound_mixer.reset();
}

std::string Fields::get_field(field target, field fields_to_update) {
    if (target == field_none) {
        return sbar::null_str;
    }

    size_t target_index = index(static_cast<size_t>(target));

    if ((target & fields_to_update) == field_none) {
        return this->values.at(target_index);
    }

    std::string value;

    switch (target) {
        case field::time:
            value = this->get_time();
            break;
        case field::uptime:
            value = this->get_uptime();
            break;
        case field::disk:
            value = this->get_disk_percent();
            break;
        case field::swap:
            value = this->get_swap_percent();
            break;
        case field::memory:
            value = this->get_memory_percent();
            break;
        case field::cpu:
            value = this->get_cpu_percent();
            break;
        case field::cpu_temp:
            value = this->get_cpu_temperature();
            break;
        case field::load_1:
            value = this->get_one_minute_load_average();
            break;
        case field::load_5:
            value = this->get_five_minute_load_average();
            break;
        case field::load_15:
            value = this->get_fifteen_minute_load_average();
            break;
        case field::battery_status:
            value = this->get_battery_status();
            break;
        case field::battery_device:
            value = this->get_battery_device();
            break;
        case field::battery:
            value = this->get_battery_percent();
            break;
        case field::battery_time:
            value = this->get_battery_time_remaining();
            break;
        case field::backlight:
            value = this->get_backlight_percent();
            break;
        case field::network_status:
            value = this->get_network_status();
            break;
        case field::network_device:
            value = this->get_network_device();
            break;
        case field::network_ssid:
            value = this->get_network_ssid();
            break;
        case field::network_strength:
            value = this->get_network_signal_strength_percent();
            break;
        case field::network_upload:
            value = this->get_network_upload();
            break;
        case field::network_download:
            value = this->get_network_download();
            break;
        case field::volume_status:
            value = this->get_volume_status();
            break;
        case field::volume:
            value = this->get_volume_percent();
            break;
        case field::capture_status:
            value = this->get_capture_status();
            break;
        case field::capture:
            value = this->get_capture_percent();
            break;
        case field::microphone:
            value = this->get_microphone_status();
            break;
        case field::camera:
            value = this->get_camera_status();
            break;
        case field::user:
            value = this->get_user();
            break;
        case field::kernel_status:
            value = this->get_outdated_kernel_indicator();
            break;
        default:
            return sbar::null_str;
    }

    this->values.at(target_index) = value;

    return value;
}

std::string Fields::format_status(Status status, field fields_to_update) {
    std::string formatted_status;

    auto fields_it = status.active_fields.begin();

    for (const auto& sep : status.separators) {
        formatted_status += sep + this->get_field(*fields_it, fields_to_update);
        fields_it++;
    }

    this->reset();

    return formatted_status;
}

} // namespace sbar
