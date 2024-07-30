#pragma once

/**
 * @file status.hpp
 * @author Caden Shmookler (cshmookler@gmail.com)
 * @brief Functions for generating the text of the status bar.
 * @date 2024-01-29
 */

// Standard includes
#include <array>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <list>
#include <memory>
#include <optional>
#include <string>
#include <vector>

// External includes
#include <alsa/asoundlib.h>

// Local includes
#include "constants.hpp"

namespace sbar {

template<typename... Args>
[[nodiscard]] std::string sprintf(const char* format, Args... args) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, hicpp-vararg)
    int size = std::snprintf(nullptr, 0, format, args...);
    std::string buffer(size, '\0');
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, hicpp-vararg)
    if (std::sprintf(buffer.data(), format, args...) < 0) {
        return error_str;
    }
    return buffer;
}

class Stopwatch {
    using System_clock = std::chrono::system_clock;
    using Time_point = System_clock::time_point;
    using Duration = System_clock::duration;

    std::string_view name_;
    Time_point start_;

  public:
    explicit Stopwatch(const std::string_view& name) : name_(name) {
        std::cout << this->name_ << " start\n";
        this->start_ = System_clock::now();
    }

    void reset() {
        Duration elapsed = System_clock::now() - this->start_;
        std::cout << this->name_ << " reset: " << std::setw(5)
                  << std::chrono::duration_cast<std::chrono::microseconds>(
                       elapsed)
                       .count()
                  << " us\n";
        this->start_ = System_clock::now();
    }
};

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

class Sound_mixer {
    static constexpr const char* default_card = "default";

    static constexpr const char* playback_name = "Master";
    static constexpr int playback_index = 0;

    static constexpr const char* capture_name = "Capture";
    static constexpr int capture_index = 0;

    static constexpr int mixer_mode = 0;

    snd_mixer_t* mixer_ = nullptr;

    bool good_ = false;

    template<typename F, typename... A>
    [[nodiscard]] static bool handle_error_(
      const std::string_view& function_name, F function, A... args) {
        static_assert(std::is_invocable_v<F, A...>);

        int errnum = function(args...);
        if (errnum == 0) {
            return false;
        }

        std::cerr << function_name << "(): " << snd_strerror(errnum) << '\n';

        return true;
    }

    [[nodiscard]] static const char* get_indicator_(int state) {
        if (state == 0) {
            return "🔴";
        }
        return "🟢";
    }

    [[nodiscard]] static long get_percent_(long min, long max, long value) {
        double ratio =
          static_cast<double>(value - min) / static_cast<double>(max);
        return static_cast<long>(ratio * 100.F);
    }

    template<typename F>
    [[nodiscard]] static std::string get_state_(
      const char* switch_function_name,
      F switch_function,
      snd_mixer_elem_t* mixer_elem) {
        int left_state = 0;
        if (handle_error_(switch_function_name,
              switch_function,
              mixer_elem,
              snd_mixer_selem_channel_id_t::SND_MIXER_SCHN_FRONT_LEFT,
              &left_state)) {
            return sbar::error_str;
        }

        int right_state = 0;
        if (handle_error_(switch_function_name,
              switch_function,
              mixer_elem,
              snd_mixer_selem_channel_id_t::SND_MIXER_SCHN_FRONT_RIGHT,
              &right_state)) {
            return sbar::error_str;
        }

        if (left_state != right_state) {
            return sprintf("(%s, %s)",
              get_indicator_(left_state),
              get_indicator_(right_state));
        }

        return get_indicator_(left_state);
    }

    template<typename R, typename V>
    [[nodiscard]] static std::string get_volume_(
      const char* volume_range_function_name,
      R volume_range_function,
      const char* volume_function_name,
      V volume_function,
      snd_mixer_elem_t* mixer_elem) {
        if (mixer_elem == nullptr) {
            return sbar::error_str;
        }

        long min_volume = -1;
        long max_volume = -1;
        if (handle_error_(volume_range_function_name,
              volume_range_function,
              mixer_elem,
              &min_volume,
              &max_volume)) {
            return sbar::error_str;
        }

        long left_volume = -1;
        if (handle_error_(volume_function_name,
              volume_function,
              mixer_elem,
              snd_mixer_selem_channel_id_t::SND_MIXER_SCHN_FRONT_LEFT,
              &left_volume)) {
            return sbar::error_str;
        }

        long right_volume = -1;
        if (handle_error_(volume_function_name,
              volume_function,
              mixer_elem,
              snd_mixer_selem_channel_id_t::SND_MIXER_SCHN_FRONT_RIGHT,
              &right_volume)) {
            return sbar::error_str;
        }

        long left_volume_percent =
          get_percent_(min_volume, max_volume, left_volume);

        long right_volume_percent =
          get_percent_(min_volume, max_volume, right_volume);

        if (left_volume != right_volume) {
            return sprintf(
              "(%i, %i)", left_volume_percent, right_volume_percent);
        }

        return sprintf("%i", left_volume_percent);
    }

    [[nodiscard]] snd_mixer_elem_t* get_mixer_elem_(
      const char* selem_name, int selem_index) const {
        if (! this->good()) {
            return nullptr;
        }

        snd_mixer_selem_id_t* selem = nullptr;
        if (handle_error_(
              "snd_mixer_selem_id_malloc", snd_mixer_selem_id_malloc, &selem)) {
            return nullptr;
        }

        snd_mixer_selem_id_set_name(selem, selem_name);
        snd_mixer_selem_id_set_index(selem, selem_index);

        return snd_mixer_find_selem(this->mixer_, selem);
    }

  public:
    Sound_mixer() {
        if (handle_error_(
              "snd_mixer_open", snd_mixer_open, &this->mixer_, mixer_mode)) {
            return;
        }
        if (handle_error_("snd_mixer_attach",
              snd_mixer_attach,
              this->mixer_,
              default_card)) {
            return;
        }
        if (handle_error_("snd_mixer_selem_register",
              snd_mixer_selem_register,
              this->mixer_,
              nullptr,
              nullptr)) {
            return;
        }
        if (handle_error_("snd_mixer_load", snd_mixer_load, this->mixer_)) {
            return;
        }
        this->good_ = true;
    }

    Sound_mixer(const Sound_mixer&) = delete;
    Sound_mixer(Sound_mixer&&) noexcept = default;
    Sound_mixer& operator=(const Sound_mixer&) = delete;
    Sound_mixer& operator=(Sound_mixer&&) noexcept = default;

    ~Sound_mixer() {
        snd_mixer_close(this->mixer_);
        // do nothing if the mixer fails to close.
    }

    [[nodiscard]] bool good() const {
        return this->good_;
    }

    [[nodiscard]] std::string get_playback_state() const {
        return get_state_("snd_mixer_selem_get_playback_switch",
          snd_mixer_selem_get_playback_switch,
          this->get_mixer_elem_(playback_name, playback_index));
    }

    [[nodiscard]] std::string get_playback_volume() const {
        return get_volume_("snd_mixer_selem_get_playback_volume_range",
          snd_mixer_selem_get_playback_volume_range,
          "snd_mixer_selem_get_playback_volume",
          snd_mixer_selem_get_playback_volume,

          this->get_mixer_elem_(playback_name, playback_index));
    }

    [[nodiscard]] std::string get_capture_state() const {
        return get_state_("snd_mixer_selem_get_capture_switch",
          snd_mixer_selem_get_capture_switch,
          this->get_mixer_elem_(capture_name, capture_index));
    }

    [[nodiscard]] std::string get_capture_volume() const {
        return get_volume_("snd_mixer_selem_get_capture_volume_range",
          snd_mixer_selem_get_capture_volume_range,
          "snd_mixer_selem_get_capture_volume",
          snd_mixer_selem_get_capture_volume,
          this->get_mixer_elem_(capture_name, capture_index));
    }
};

[[nodiscard]] std::string get_volume_state(const Sound_mixer& mixer);

[[nodiscard]] std::string get_volume_perc(const Sound_mixer& mixer);

[[nodiscard]] std::string get_capture_state(const Sound_mixer& mixer);

[[nodiscard]] std::string get_capture_perc(const Sound_mixer& mixer);

[[nodiscard]] std::string get_microphone_state();

[[nodiscard]] std::string get_camera_state();

[[nodiscard]] std::string get_user();

[[nodiscard]] std::string get_outdated_kernel_indicator();

} // namespace sbar
