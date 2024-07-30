#pragma once

/**
 * @file status.hpp
 * @author Caden Shmookler (cshmookler@gmail.com)
 * @brief Functions for generating the text of the status bar.
 * @date 2024-01-29
 */

// Standard includes
#include <array>
#include <filesystem>
#include <list>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace sbar {

[[nodiscard]] std::string get_time();

[[nodiscard]] std::string get_uptime();

[[nodiscard]] std::string get_disk_percent();

[[nodiscard]] std::string get_memory_percent();

[[nodiscard]] std::string get_swap_percent();

struct Cpu_state {
    // clang-format off
  private:
    static const size_t index_start = __LINE__;
  public:
    enum class Index : size_t {
        user_mode,
        low_priority_user_mode,
        system_mode,
        idle,
        io_idle,
        interrupt,
        soft_interrupt,
        stolen,
        guest,
        niced_guest,
    };
    static const size_t index_count = __LINE__ - index_start - 4;
    // clang-format on

  private:
    std::array<size_t, index_count> entries_;

  public:
    [[nodiscard]] bool update();

    [[nodiscard]] size_t get_total() const;

    [[nodiscard]] size_t get_total(const std::vector<Index>& indicies) const;
};

[[nodiscard]] std::string get_cpu_percent(
  std::unique_ptr<Cpu_state>& cpu_state_info);

[[nodiscard]] std::string get_cpu_temperature();

[[nodiscard]] std::string get_one_minute_load_average();

[[nodiscard]] std::string get_five_minute_load_average();

[[nodiscard]] std::string get_fifteen_minute_load_average();

[[nodiscard]] std::optional<std::filesystem::path> get_battery();

[[nodiscard]] std::string get_battery_status(
  const std::filesystem::path& battery_path);

[[nodiscard]] std::string get_battery_device(
  const std::filesystem::path& battery_path);

[[nodiscard]] std::string get_battery_percent(
  const std::filesystem::path& battery_path);

struct Battery_state {
    static const size_t sample_size = 60;

  private:
    std::list<size_t> energy_remaining_;

  public:
    [[nodiscard]] bool add_sample(const std::filesystem::path& battery_path);

    [[nodiscard]] bool has_enough_samples() const;

    [[nodiscard]] std::string get_time_remaining() const;
};

[[nodiscard]] std::string get_battery_time_remaining(
  const std::filesystem::path& battery_path, Battery_state& battery_state_info);

[[nodiscard]] std::string get_backlight_percent();

[[nodiscard]] std::optional<std::filesystem::path> get_network();

[[nodiscard]] std::string get_network_status(
  const std::filesystem::path& network_interface_path);

[[nodiscard]] std::string get_network_device(
  const std::filesystem::path& network_interface_path);

[[nodiscard]] std::string get_network_ssid(
  const std::filesystem::path& network_interface_path);

[[nodiscard]] std::string get_network_signal_strength_percent(
  const std::filesystem::path& network_interface_path);

struct Network_data_stats {
  private:
    size_t upload_byte_count_ = 0;
    size_t download_byte_count_ = 0;

  public:
    size_t get_upload_byte_difference(size_t upload_byte_count);

    size_t get_download_byte_difference(size_t download_byte_count);
};

[[nodiscard]] std::string get_network_upload(
  const std::filesystem::path& network_interface_path,
  Network_data_stats& network_state_info);

[[nodiscard]] std::string get_network_download(
  const std::filesystem::path& network_interface_path,
  Network_data_stats& network_state_info);

[[nodiscard]] std::string get_volume_state();

[[nodiscard]] std::string get_volume_perc();

[[nodiscard]] std::string get_capture_state();

[[nodiscard]] std::string get_capture_perc();

[[nodiscard]] std::string get_microphone_state();

[[nodiscard]] std::string get_camera_state();

[[nodiscard]] std::string get_user();

[[nodiscard]] std::string get_outdated_kernel_indicator();

} // namespace sbar
