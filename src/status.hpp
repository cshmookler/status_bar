#pragma once

/**
 * @file status.hpp
 * @author Caden Shmookler (cshmookler@gmail.com)
 * @brief Functions for generating the text of the status bar.
 * @date 2024-01-29
 */

// Standard includes
#include <memory>
#include <string>

// Local includes
#include "proc_stat.hpp"

namespace status_bar {

[[nodiscard]] std::string time();

[[nodiscard]] std::string uptime();

[[nodiscard]] std::string disk_percent();

[[nodiscard]] std::string memory_percent();

[[nodiscard]] std::string swap_percent();

[[nodiscard]] std::string cpu_percent(std::unique_ptr<cpu>& cpu_stat);

[[nodiscard]] std::string cpu_temp();

[[nodiscard]] std::string battery_state();

[[nodiscard]] std::string battery_perc();

[[nodiscard]] std::string backlight_perc();

[[nodiscard]] std::string network_ssid();

[[nodiscard]] std::string wifi_perc();

[[nodiscard]] std::string bluetooth_devices();

[[nodiscard]] std::string volume_status();

[[nodiscard]] std::string volume_perc();

[[nodiscard]] std::string microphone_state();

[[nodiscard]] std::string camera_state();

} // namespace status_bar
