#pragma once

/**
 * @file persistent.hpp
 * @author Caden Shmookler (cshmookler@gmail.com)
 * @brief Structures for storing persistent status information.
 * @date 2024-08-12
 */

// Standard includes
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <list>
#include <string>

// External includes
#include <alsa/asoundlib.h>
#include <sys/sysinfo.h>

// Local includes
#include "constants.hpp"
#include "utils.hpp"
#include "../include/notify.hpp"

namespace sbar {

class System {
    struct sysinfo info_ {};
    bool good_ = false;

  public:
    bool init() {
        this->good_ = sysinfo(&this->info_) == 0;
        return this->good_;
    }

    inline void reset() {
        this->good_ = false;
    }

    [[nodiscard]] inline bool good() const {
        return this->good_;
    }

    [[nodiscard]] inline const struct sysinfo& info() const {
        return this->info_;
    }

    [[nodiscard]] inline const struct sysinfo* operator->() const {
        return &this->info_;
    }
};

// clang-format off
namespace {
constexpr size_t cpu_stat_first_line = __LINE__ + 4;
} // namespace
// Do NOT change the spacing here!
enum class cpu_stat : size_t {
    user_mode = bit(__LINE__ - cpu_stat_first_line),
    low_priority_user_mode = bit(__LINE__ - cpu_stat_first_line),
    system_mode = bit(__LINE__ - cpu_stat_first_line),
    idle = bit(__LINE__ - cpu_stat_first_line),
    io_idle = bit(__LINE__ - cpu_stat_first_line),
    interrupt = bit(__LINE__ - cpu_stat_first_line),
    soft_interrupt = bit(__LINE__ - cpu_stat_first_line),
    stolen = bit(__LINE__ - cpu_stat_first_line),
    guest = bit(__LINE__ - cpu_stat_first_line),
    niced_guest = bit(__LINE__ - cpu_stat_first_line),
};
// Do NOT change the spacing here!
namespace {
constexpr size_t cpu_stat_last_line = __LINE__ - 4;
} // namespace
// clang-format on

constexpr size_t cpu_stat_count = cpu_stat_last_line - cpu_stat_first_line + 1;

constexpr cpu_stat cpu_stat_none = static_cast<cpu_stat>(0);
constexpr cpu_stat cpu_stat_all =
  static_cast<cpu_stat>(bit(cpu_stat_count) - 1);

inline constexpr cpu_stat operator|(cpu_stat lhs, size_t rhs) {
    return static_cast<cpu_stat>(static_cast<size_t>(lhs) | rhs);
}
inline constexpr cpu_stat operator|(size_t lhs, cpu_stat rhs) {
    return operator|(rhs, lhs);
}
inline constexpr cpu_stat operator|(cpu_stat lhs, cpu_stat rhs) {
    return operator|(lhs, static_cast<size_t>(rhs));
}
inline constexpr cpu_stat operator&(cpu_stat lhs, size_t rhs) {
    return static_cast<cpu_stat>(static_cast<size_t>(lhs) & rhs);
}
inline constexpr cpu_stat operator&(size_t lhs, cpu_stat rhs) {
    return operator&(rhs, lhs);
}
inline constexpr cpu_stat operator&(cpu_stat lhs, cpu_stat rhs) {
    return operator&(lhs, static_cast<size_t>(rhs));
}

class Cpu {
    std::array<size_t, cpu_stat_count> stat_{};
    bool ready_ = false;

  public:
    [[nodiscard]] inline bool ready() const {
        return this->ready_;
    }

    bool update_stat();

    [[nodiscard]] size_t get_total() const;

    [[nodiscard]] size_t get_total(cpu_stat stat) const;
};

class Battery {
    std::filesystem::path path_;
    bool good_ = false;

    std::list<size_t> energy_remaining_;

  public:
    static const size_t sample_size = 60;

    bool init();

    inline void reset() {
        this->good_ = false;
    }

    [[nodiscard]] inline bool good() const {
        return this->good_;
    }

    [[nodiscard]] inline const std::filesystem::path& path() const {
        return this->path_;
    }

    [[nodiscard]] inline const std::filesystem::path* operator->() const {
        return &this->path_;
    }

    [[nodiscard]] bool add_sample();

    [[nodiscard]] bool has_enough_samples() const;

    [[nodiscard]] std::string get_time_remaining() const;
};

class Backlight {
    std::filesystem::path path_;
    bool good_ = false;

  public:
    bool init();

    inline void reset() {
        this->good_ = false;
    }

    [[nodiscard]] inline bool good() const {
        return this->good_;
    }

    [[nodiscard]] inline const std::filesystem::path& path() const {
        return this->path_;
    }

    [[nodiscard]] inline const std::filesystem::path* operator->() const {
        return &this->path_;
    }
};

class Network {
    std::filesystem::path path_;
    bool good_ = false;

    size_t upload_byte_count_ = 0;
    size_t download_byte_count_ = 0;

  public:
    bool init();

    inline void reset() {
        this->good_ = false;
    }

    [[nodiscard]] inline bool good() const {
        return this->good_;
    }

    [[nodiscard]] inline const std::filesystem::path& path() const {
        return this->path_;
    }

    [[nodiscard]] inline const std::filesystem::path* operator->() const {
        return &this->path_;
    }

    size_t get_upload_byte_difference(size_t upload_byte_count);

    size_t get_download_byte_difference(size_t download_byte_count);
};

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
    [[nodiscard]] static std::string get_status_(
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
    Sound_mixer() = default;
    Sound_mixer(const Sound_mixer&) = delete;
    Sound_mixer(Sound_mixer&& sound_mixer) noexcept
    : mixer_(sound_mixer.mixer_), good_(sound_mixer.good_) {
        sound_mixer.mixer_ = nullptr;
        sound_mixer.good_ = false;
    }
    Sound_mixer& operator=(const Sound_mixer&) = delete;
    Sound_mixer& operator=(Sound_mixer&& sound_mixer) noexcept = default;
    ~Sound_mixer() {
        this->reset();
    }

    bool init() {
        this->reset();
        if (handle_error_(
              "snd_mixer_open", snd_mixer_open, &this->mixer_, mixer_mode)) {
            return false;
        }
        if (handle_error_("snd_mixer_attach",
              snd_mixer_attach,
              this->mixer_,
              default_card)) {
            return false;
        }
        if (handle_error_("snd_mixer_selem_register",
              snd_mixer_selem_register,
              this->mixer_,
              nullptr,
              nullptr)) {
            return false;
        }
        if (handle_error_("snd_mixer_load", snd_mixer_load, this->mixer_)) {
            return false;
        }
        this->good_ = true;
        return true;
    }

    void reset() {
        this->good_ = false;
        if (this->mixer_ != nullptr) {
            snd_mixer_close(this->mixer_);
            this->mixer_ = nullptr;
            // do nothing if the mixer fails to close.
        }
    }

    [[nodiscard]] inline bool good() const {
        return this->good_;
    }

    [[nodiscard]] std::string get_playback_status() const {
        return get_status_("snd_mixer_selem_get_playback_switch",
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

    [[nodiscard]] std::string get_capture_status() const {
        return get_status_("snd_mixer_selem_get_capture_switch",
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

} // namespace sbar
