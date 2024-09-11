#pragma once

/**
 * @file status.hpp
 * @author Caden Shmookler (cshmookler@gmail.com)
 * @brief Functions for generating the text of the status bar.
 * @date 2024-01-29
 */

// Standard includes
#include <string>
#include <vector>

// Local includes
#include "persistent.hpp"

namespace sbar {

struct Status {
    std::vector<sbar::field> active_fields;
    std::vector<std::string> separators;
};

struct Fields {
    std::array<std::string, field_count> values{};
    System system;
    Cpu cpu;
    Battery battery;
    Backlight backlight;
    Network network;
    Sound_mixer sound_mixer;

    void reset();
    [[nodiscard]] std::string get_field(field target, field fields_to_update);
    [[nodiscard]] std::string format_status(
      Status status, field fields_to_update);

    [[nodiscard]] std::string get_time();
    [[nodiscard]] std::string get_uptime();
    [[nodiscard]] std::string get_disk_percent();
    [[nodiscard]] std::string get_memory_percent();
    [[nodiscard]] std::string get_swap_percent();
    [[nodiscard]] std::string get_cpu_percent();
    [[nodiscard]] std::string get_cpu_temperature();
    [[nodiscard]] std::string get_one_minute_load_average();
    [[nodiscard]] std::string get_five_minute_load_average();
    [[nodiscard]] std::string get_fifteen_minute_load_average();
    [[nodiscard]] std::string get_battery_status();
    [[nodiscard]] std::string get_battery_device();
    [[nodiscard]] std::string get_battery_percent();
    [[nodiscard]] std::string get_battery_time_remaining();
    [[nodiscard]] std::string get_backlight_percent();
    [[nodiscard]] std::string get_network_status();
    [[nodiscard]] std::string get_network_device();
    [[nodiscard]] std::string get_network_ssid();
    [[nodiscard]] std::string get_network_signal_strength_percent();
    [[nodiscard]] std::string get_network_upload();
    [[nodiscard]] std::string get_network_download();
    [[nodiscard]] std::string get_volume_status();
    [[nodiscard]] std::string get_volume_percent();
    [[nodiscard]] std::string get_capture_status();
    [[nodiscard]] std::string get_capture_percent();
    [[nodiscard]] std::string get_microphone_status();
    [[nodiscard]] std::string get_camera_status();
    [[nodiscard]] std::string get_user();
    [[nodiscard]] std::string get_outdated_kernel_indicator();
};

} // namespace sbar
