#pragma once

/**
 * @file status.hpp
 * @author Caden Shmookler (cshmookler@gmail.com)
 * @brief Functions for generating the text of the status bar.
 * @date 2024-01-29
 */

// Standard includes
#include <string>

// Local includes
#include "persistent.hpp"

namespace sbar {

[[nodiscard]] std::string get_time();

[[nodiscard]] std::string get_uptime(System& system);

[[nodiscard]] std::string get_disk_percent();

[[nodiscard]] std::string get_memory_percent(System& system);

[[nodiscard]] std::string get_swap_percent(System& system);

[[nodiscard]] std::string get_cpu_percent(Cpu& cpu);

[[nodiscard]] std::string get_cpu_temperature();

[[nodiscard]] std::string get_one_minute_load_average(System& system);

[[nodiscard]] std::string get_five_minute_load_average(System& system);

[[nodiscard]] std::string get_fifteen_minute_load_average(System& system);

[[nodiscard]] std::string get_battery_status(Battery& battery_path);

[[nodiscard]] std::string get_battery_device(Battery& battery_path);

[[nodiscard]] std::string get_battery_percent(Battery& battery_path);

[[nodiscard]] std::string get_battery_time_remaining(Battery& battery);

[[nodiscard]] std::string get_backlight_percent(Backlight& backlight);

[[nodiscard]] std::string get_network_status(Network& network);

[[nodiscard]] std::string get_network_device(Network& network);

[[nodiscard]] std::string get_network_ssid(Network& network);

[[nodiscard]] std::string get_network_signal_strength_percent(Network& network);

[[nodiscard]] std::string get_network_upload(Network& network);

[[nodiscard]] std::string get_network_download(Network& network);

[[nodiscard]] std::string get_volume_status(Sound_mixer& mixer);

[[nodiscard]] std::string get_volume_perc(Sound_mixer& mixer);

[[nodiscard]] std::string get_capture_status(Sound_mixer& mixer);

[[nodiscard]] std::string get_capture_perc(Sound_mixer& mixer);

[[nodiscard]] std::string get_microphone_status();

[[nodiscard]] std::string get_camera_status();

[[nodiscard]] std::string get_user();

[[nodiscard]] std::string get_outdated_kernel_indicator();

} // namespace sbar
