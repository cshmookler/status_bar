// Standard includes
#include <filesystem>
#include <iostream>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <optional>

// External includes
#include <argparse/argparse.hpp>
#include <cpp_result/all.hpp>
#include <inotify_ipc/iipc.hpp>
#include <system_state/system_state.hpp>

// Local includes
#include "../build/version.h"
#include "root_window.hpp"
#include "../include/notify.h"
#include "channel.hpp"

const std::string error_status = "❌";

bool keep_running = true;

void signal_handler(int signal) {
    keep_running = false;
}

/**
 * @brief Attempts to format a given string using std::sprintf.
 * Returns an error if the formatting fails.
 *
 * @tparam Args - The types of the arguments for formatting method.
 * @param[in] format - The string to be formatted.
 * @param[in] args - The arguments for the formatting method.
 */
template<typename... args_t>
[[nodiscard]] res::optional_t<std::string> sprintf(
  const char* format, args_t... args) {
    int size = std::snprintf(nullptr, 0, format, args...);

    std::string buffer(size, '\0');
    if (std::sprintf(buffer.data(), format, args...) < 0) {
        return RES_NEW_ERROR(
          std::string{ "Failed to format the given string.\n\tstring: " }
          + format);
    }

    return buffer;
}

res::optional_t<std::string> add_storage_size_unit(uint64_t size) {
    const uint64_t kibibyte = 1024;
    const uint64_t mebibyte = kibibyte * 1024;
    const uint64_t gibibyte = mebibyte * 1024;
    const uint64_t tebibyte = gibibyte * 1024;
    const uint64_t pebibyte = tebibyte * 1024;

    if (size > pebibyte) {
        return sprintf("%iP", size / pebibyte);
    }
    if (size > tebibyte) {
        return sprintf("%iT", size / tebibyte);
    }
    if (size > gibibyte) {
        return sprintf("%iG", size / gibibyte);
    }
    if (size > mebibyte) {
        return sprintf("%iM", size / mebibyte);
    }
    if (size > kibibyte) {
        return sprintf("%iK", size / kibibyte);
    }
    return sprintf("%i", size);
}

struct persistent_state_t {
    // format strings
    std::string status_fmt;
    std::string disk_fmt;
    std::string part_fmt;
    std::string backlight_fmt;
    std::string battery_fmt;
    std::string network_fmt;
    std::string audio_playback_fmt;
    std::string audio_capture_fmt;

    // options
    bool ignore_zero_capacity_disks = true;

    // persistent system_info structures
    std::optional<syst::system_info_t> system_info;
    std::optional<std::reference_wrapper<syst::sound_mixer_t>> sound_mixer;
    syst::cpu_usage_t cpu_usage;

    // fields to update
    sbar_field_t fields_to_update = sbar_field_all;

    // saved field values
    std::vector<std::string> fields =
      std::vector<std::string>(sbar_total_fields);
};

using field_assigner_t = sbar_field_t (*)(char);

template<typename... generator_args_t>
using field_generator_t = res::optional_t<std::string> (*)(
  sbar_field_t, persistent_state_t&, generator_args_t...);

template<typename... field_generator_args_t>
[[nodiscard]] std::string make_given_status(const std::string& fmt,
  persistent_state_t& persistent_state,
  field_assigner_t field_assigner,
  field_generator_t<const field_generator_args_t&...> generator,
  const field_generator_args_t&... generator_args) {
    const char escape_seq = '/';

    std::string status;

    bool escaped = false;
    for (char chr : fmt) {
        if (! escaped) {
            if (chr == escape_seq) {
                escaped = true;
            } else {
                status += chr;
            }
            continue;
        }

        if (chr == escape_seq) {
            status += chr;
            continue;
        }

        escaped = false;

        sbar_field_t field = field_assigner(chr);
        if (field == sbar_field_none) {
            std::cerr << "Invalid escaped token: '" << escape_seq << chr << "'"
                      << std::endl;
            continue;
        }

        size_t field_index = __builtin_ctzll(field);

        if ((field & persistent_state.fields_to_update) == sbar_field_none) {
            status += persistent_state.fields.at(field_index);
            continue;
        }

        auto result = generator(field,
          std::forward<persistent_state_t&>(persistent_state),
          std::forward<const field_generator_args_t&>(generator_args)...);

        std::string status_part;
        if (result.has_value()) {
            status_part = result.value();
        } else {
            status_part = error_status;
            std::cerr << result.error() << std::endl;
        }

        persistent_state.fields.at(field_index) = status_part;
        status += status_part;
    }

    return status;
}

[[nodiscard]] sbar_field_t part_field_assigner(char token) {
    switch (token) {
        case 'N':
            return sbar_field_part_name;
        case 'R':
            return sbar_field_part_read_only;
        case 'M':
            return sbar_field_part_mount;
        case 'T':
            return sbar_field_part_filesystem;
        case 'C':
            return sbar_field_part_size;
        case 'U':
            return sbar_field_part_usage;
        case 'F':
            return sbar_field_part_in_flight;
        default:
            return sbar_field_none;
    }
}

[[nodiscard]] res::optional_t<std::string> part_field_generator(
  sbar_field_t field,
  persistent_state_t& persistent_state,
  const syst::part_t& part) {
    switch (field) {
        case sbar_field_part_name: {
            return part.get_name();
        }
        case sbar_field_part_read_only: {
            auto read_only = part.is_read_only();
            if (! read_only.has_value()) {
                return RES_TRACE(read_only.error());
            }

            if (read_only.value()) {
                return std::string{ "\U0001F441" }; // 👁️
            }

            return std::string{ "\U0000270F" }; // ✏️
        }
        case sbar_field_part_mount: {
            auto mount_info = part.get_mount_info();
            if (! mount_info.has_value()) {
                return RES_TRACE(mount_info.error());
            }

            return mount_info->mount_path.string();
        }
        case sbar_field_part_filesystem: {
            auto mount_info = part.get_mount_info();
            if (! mount_info.has_value()) {
                return RES_TRACE(mount_info.error());
            }

            return mount_info->fs_type;
        }
        case sbar_field_part_size: {
            auto size = part.get_size();
            if (! size.has_value()) {
                return RES_TRACE(size.error());
            }

            return add_storage_size_unit(size.value());
        }
        case sbar_field_part_usage: {
            auto mount_info = part.get_mount_info();
            if (! mount_info.has_value()) {
                return RES_TRACE(mount_info.error());
            }

            auto space_info = std::filesystem::space(mount_info->mount_path);

            auto available = static_cast<double>(space_info.available);
            auto capacity = static_cast<double>(space_info.capacity);
            auto used = 100 * (1 - (available / capacity));

            return sprintf("%.0f", used);
        }
        case sbar_field_part_in_flight: {
            auto io_stat = part.get_io_stat();
            if (! io_stat.has_value()) {
                return RES_TRACE(io_stat.error());
            }

            return sprintf("%i", io_stat->io_in_flight);
        }
        default:
            return RES_NEW_ERROR(
              "Invalid field value: " + std::to_string(field));
    }
}

[[nodiscard]] sbar_field_t disk_field_assigner(char token) {
    switch (token) {
        case 'N':
            return sbar_field_disk_name;
        case 'O':
            return sbar_field_disk_rotational;
        case 'R':
            return sbar_field_disk_read_only;
        case 'E':
            return sbar_field_disk_removable;
        case 'C':
            return sbar_field_disk_size;
        case 'F':
            return sbar_field_disk_in_flight;
        case 'P':
            return sbar_field_part;
        default:
            return sbar_field_none;
    }
}

[[nodiscard]] res::optional_t<std::string> disk_field_generator(
  sbar_field_t field,
  persistent_state_t& persistent_state,
  const syst::disk_t& disk) {
    switch (field) {
        case sbar_field_disk_name: {
            return disk.get_name();
        }
        case sbar_field_disk_rotational: {
            auto rotational = disk.is_rotational();
            if (! rotational.has_value()) {
                return RES_TRACE(rotational.error());
            }

            if (rotational.value()) {
                return std::string{ "💿" };
            }

            return std::string{ "💾" };
        }
        case sbar_field_disk_read_only: {
            auto read_only = disk.is_read_only();
            if (! read_only.has_value()) {
                return RES_TRACE(read_only.error());
            }

            if (read_only.value()) {
                return std::string{ "\U0001F441" }; // 👁️
            }

            return std::string{ "\U0000270F" }; // ✏️
        }
        case sbar_field_disk_removable: {
            auto removable = disk.is_removable();
            if (! removable.has_value()) {
                return RES_TRACE(removable.error());
            }

            if (removable.value()) {
                return std::string{ "🔌" };
            }

            return std::string{ "" };
        }
        case sbar_field_disk_size: {
            auto size = disk.get_size();
            if (! size.has_value()) {
                return RES_TRACE(size.error());
            }

            return add_storage_size_unit(size.value());
        }
        case sbar_field_disk_in_flight: {
            auto io_stat = disk.get_io_stat();
            if (! io_stat.has_value()) {
                return RES_TRACE(io_stat.error());
            }

            return sprintf("%i", io_stat->io_in_flight);
        }
        case sbar_field_part: {
            auto parts = disk.get_parts();
            if (parts.has_error()) {
                return RES_TRACE(parts.error());
            }

            std::string status;

            for (const auto& part : parts.value()) {
                status += make_given_status(persistent_state.part_fmt,
                  persistent_state,
                  part_field_assigner,
                  part_field_generator,
                  part);
            }

            return status;
        }
        default:
            return RES_NEW_ERROR(
              "Invalid field value: " + std::to_string(field));
    }
}

[[nodiscard]] sbar_field_t backlight_field_assigner(char token) {
    switch (token) {
        case 'N':
            return sbar_field_backlight_name;
        case 'L':
            return sbar_field_backlight_brightness;
        default:
            return sbar_field_none;
    }
}

[[nodiscard]] res::optional_t<std::string> backlight_field_generator(
  sbar_field_t field,
  persistent_state_t& persistent_state,
  const syst::backlight_t& backlight) {
    switch (field) {
        case sbar_field_backlight_name: {
            return backlight.get_name();
        }
        case sbar_field_backlight_brightness: {
            auto brightness = backlight.get_brightness();
            if (brightness.has_error()) {
                return RES_TRACE(brightness.error());
            }
            return sprintf("%i", static_cast<int>(brightness.value()));
        }
        default:
            return RES_NEW_ERROR(
              "Invalid field value: " + std::to_string(field));
    }
}

[[nodiscard]] sbar_field_t battery_field_assigner(char token) {
    switch (token) {
        case 'N':
            return sbar_field_battery_name;
        case 'S':
            return sbar_field_battery_status;
        case 'L':
            return sbar_field_battery_charge;
        case 'C':
            return sbar_field_battery_capacity;
        case 'I':
            return sbar_field_battery_current;
        case 'P':
            return sbar_field_battery_power;
        case 'T':
            return sbar_field_battery_time;
        default:
            return sbar_field_none;
    }
}

[[nodiscard]] res::optional_t<std::string> battery_field_generator(
  sbar_field_t field,
  persistent_state_t& persistent_state,
  const syst::battery_t& battery) {
    switch (field) {
        case sbar_field_battery_name: {
            return battery.get_name();
        }
        case sbar_field_battery_status: {
            auto status = battery.get_status();
            if (status.has_error()) {
                return RES_TRACE(status.error());
            }

            if (status.value() == syst::battery_t::status_t::full
              || status.value() == syst::battery_t::status_t::charging) {
                return std::string{ "🟢" };
            }
            if (status.value() == syst::battery_t::status_t::not_charging) {
                return std::string{ "⭕" };
            }
            if (status.value() != syst::battery_t::status_t::discharging) {
                return RES_NEW_ERROR("Unknown battery status code: "
                  + std::to_string(static_cast<int>(status.value())));
            }

            auto charge = battery.get_charge();
            if (charge.has_error()) {
                return RES_TRACE(charge.error());
            }

            const double very_low_charge = 20;
            const double low_charge = 40;
            const double medium_charge = 60;

            if (charge.value() <= very_low_charge) {
                return std::string{ "🔴" };
            }
            if (charge.value() <= low_charge) {
                return std::string{ "🟠" };
            }
            if (charge.value() <= medium_charge) {
                return std::string{ "🟡" };
            }
            return std::string{ "🔵" };
        }
        case sbar_field_battery_charge: {
            auto charge = battery.get_charge();
            if (charge.has_error()) {
                return RES_TRACE(charge.error());
            }
            return sprintf("%i", static_cast<int>(charge.value()));
        }
        case sbar_field_battery_capacity: {
            auto capacity = battery.get_capacity();
            if (capacity.has_error()) {
                return RES_TRACE(capacity.error());
            }
            return sprintf("%f", capacity.value());
        }
        case sbar_field_battery_current: {
            auto current = battery.get_current();
            if (current.has_error()) {
                return RES_TRACE(current.error());
            }
            return sprintf("%f", current.value());
        }
        case sbar_field_battery_power: {
            auto power = battery.get_power();
            if (power.has_error()) {
                return RES_TRACE(power.error());
            }
            return sprintf("%f", power.value());
        }
        case sbar_field_battery_time: {
            auto status = battery.get_status();
            if (status.has_error()) {
                return RES_TRACE(status.error());
            }

            if (status.value() != syst::battery_t::status_t::charging
              && status.value() != syst::battery_t::status_t::discharging) {
                return std::string{ "⭕" };
            }

            auto seconds = battery.get_time_remaining();
            if (seconds.has_error()) {
                return RES_TRACE(seconds.error());
            }

            auto hours = seconds.value().count() / 3600;
            auto minutes = (seconds.value().count() % 3600) / 60;

            return sprintf("%i:%.2i", hours, minutes);
        }
        default:
            return RES_NEW_ERROR(
              "Invalid field value: " + std::to_string(field));
    }
}

[[nodiscard]] sbar_field_t network_field_assigner(char token) {
    switch (token) {
        case 'N':
            return sbar_field_network_name;
        case 'S':
            return sbar_field_network_status;
        case 'R':
            return sbar_field_network_packets_down;
        case 'T':
            return sbar_field_network_packets_up;
        case 'r':
            return sbar_field_network_bytes_down;
        case 't':
            return sbar_field_network_bytes_up;
        default:
            return sbar_field_none;
    }
}

[[nodiscard]] res::optional_t<std::string> network_field_generator(
  sbar_field_t field,
  persistent_state_t& persistent_state,
  const syst::network_interface_t& network_interface) {
    switch (field) {
        case sbar_field_network_name: {
            return network_interface.get_name();
        }
        case sbar_field_network_status: {
            auto status = network_interface.get_status();
            if (status.has_error()) {
                return RES_TRACE(status.error());
            }

            if (status.value() == syst::network_interface_t::status_t::up) {
                return std::string{ "🟢" };
            }
            if (status.value()
              == syst::network_interface_t::status_t::dormant) {
                return std::string{ "🟡" };
            }
            if (status.value() == syst::network_interface_t::status_t::down) {
                return std::string{ "🔴" };
            }

            return RES_NEW_ERROR("Unknown network interface status code: "
              + std::to_string(static_cast<int>(status.value())));
        }
        case sbar_field_network_packets_down: {
            auto stat = network_interface.get_stat();
            if (stat.has_error()) {
                return RES_TRACE(stat.error());
            }
            return std::to_string(stat->packets_down);
        }
        case sbar_field_network_packets_up: {
            auto stat = network_interface.get_stat();
            if (stat.has_error()) {
                return RES_TRACE(stat.error());
            }
            return std::to_string(stat->packets_up);
        }
        case sbar_field_network_bytes_down: {
            auto stat = network_interface.get_stat();
            if (stat.has_error()) {
                return RES_TRACE(stat.error());
            }
            return std::to_string(stat->bytes_down);
        }
        case sbar_field_network_bytes_up: {
            auto stat = network_interface.get_stat();
            if (stat.has_error()) {
                return RES_TRACE(stat.error());
            }
            return std::to_string(stat->bytes_up);
        }
        default:
            return RES_NEW_ERROR(
              "Invalid field value: " + std::to_string(field));
    }
}

[[nodiscard]] std::string audio_channel_status_to_string(bool status) {
    if (status) {
        return "🟢";
    }

    return "🔴";
}

[[nodiscard]] std::string make_audio_channel_status(
  const std::optional<bool>& status,
  const std::string& label,
  std::optional<bool>& first_value,
  bool& all_values_match) {
    if (! status.has_value()) {
        return "";
    }

    if (first_value.has_value()) {
        if (status.value() != first_value.value()) {
            all_values_match = false;
        }
    } else {
        first_value = status;
    }

    return audio_channel_status_to_string(status.value()) + label + ' ';
}

[[nodiscard]] std::string audio_channel_volume_to_string(double status) {
    auto result = sprintf("%i", static_cast<int>(status));
    if (result.has_error()) {
        std::cerr << RES_TRACE(result.error()) << std::endl;
        return error_status;
    }
    return result.value();
}

[[nodiscard]] std::string make_audio_channel_volume(
  const std::optional<double>& volume,
  const std::string& label,
  std::optional<double>& first_value,
  bool& all_values_match) {
    if (! volume.has_value()) {
        return "";
    }

    if (first_value.has_value()) {
        if (volume.value() != first_value.value()) {
            all_values_match = false;
        }
    } else {
        first_value = volume;
    }

    return audio_channel_volume_to_string(volume.value()) + label + ' ';
}

[[nodiscard]] sbar_field_t audio_playback_field_assigner(char token) {
    switch (token) {
        case 'N':
            return sbar_field_audio_playback_name;
        case 'S':
            return sbar_field_audio_playback_status;
        case 'V':
            return sbar_field_audio_playback_volume;
        default:
            return sbar_field_none;
    }
}

[[nodiscard]] res::optional_t<std::string> audio_playback_field_generator(
  sbar_field_t field,
  persistent_state_t& persistent_state,
  const syst::sound_control_t& sound_control) {
    switch (field) {
        case sbar_field_audio_playback_name: {
            return sound_control.get_name();
        }
        case sbar_field_audio_playback_status: {
            if (! sound_control.has_playback_status()) {
                return RES_NEW_ERROR("This audio control does not have a "
                                     "playback status.\n\tname: "
                  + sound_control.get_name());
            }

            auto playback_status = sound_control.get_playback_status();
            if (playback_status.has_error()) {
                return RES_TRACE(playback_status.error());
            }

            std::string status;
            std::optional<bool> first;
            bool all_match = true;

            status += '(';

            status += make_audio_channel_status(
              playback_status->front_left, "fl", first, all_match);
            status += make_audio_channel_status(
              playback_status->front_center, "fc", first, all_match);
            status += make_audio_channel_status(
              playback_status->front_right, "fr", first, all_match);
            status += make_audio_channel_status(
              playback_status->side_left, "sl", first, all_match);
            status += make_audio_channel_status(
              playback_status->woofer, "w", first, all_match);
            status += make_audio_channel_status(
              playback_status->side_right, "sr", first, all_match);
            status += make_audio_channel_status(
              playback_status->rear_left, "rl", first, all_match);
            status += make_audio_channel_status(
              playback_status->rear_center, "rc", first, all_match);
            status += make_audio_channel_status(
              playback_status->rear_right, "rr", first, all_match);

            if (! first.has_value()) {
                return std::string{};
            }

            if (all_match) {
                return audio_channel_status_to_string(first.value());
            }

            status.back() = ')';

            return status;
        }
        case sbar_field_audio_playback_volume: {
            if (! sound_control.has_playback_volume()) {
                return RES_NEW_ERROR("This audio control does not have a "
                                     "playback volume.\n\tname: "
                  + sound_control.get_name());
            }

            auto playback_volume = sound_control.get_playback_volume();
            if (playback_volume.has_error()) {
                return RES_TRACE(playback_volume.error());
            }

            std::string status;
            std::optional<double> first;
            bool all_match = true;

            status += '(';

            status += make_audio_channel_volume(
              playback_volume->front_left, "fl", first, all_match);
            status += make_audio_channel_volume(
              playback_volume->front_center, "fc", first, all_match);
            status += make_audio_channel_volume(
              playback_volume->front_right, "fr", first, all_match);
            status += make_audio_channel_volume(
              playback_volume->side_left, "sl", first, all_match);
            status += make_audio_channel_volume(
              playback_volume->woofer, "w", first, all_match);
            status += make_audio_channel_volume(
              playback_volume->side_right, "sr", first, all_match);
            status += make_audio_channel_volume(
              playback_volume->rear_left, "rl", first, all_match);
            status += make_audio_channel_volume(
              playback_volume->rear_center, "rc", first, all_match);
            status += make_audio_channel_volume(
              playback_volume->rear_right, "rr", first, all_match);

            if (! first.has_value()) {
                return std::string{};
            }

            if (all_match) {
                return audio_channel_volume_to_string(first.value());
            }

            status.back() = ')';

            return status;
        }
        default:
            return RES_NEW_ERROR(
              "Invalid field value: " + std::to_string(field));
    }
}

[[nodiscard]] sbar_field_t audio_capture_field_assigner(char token) {
    switch (token) {
        case 'N':
            return sbar_field_audio_capture_name;
        case 'S':
            return sbar_field_audio_capture_status;
        case 'V':
            return sbar_field_audio_capture_volume;
        default:
            return sbar_field_none;
    }
}

[[nodiscard]] res::optional_t<std::string> audio_capture_field_generator(
  sbar_field_t field,
  persistent_state_t& persistent_state,
  const syst::sound_control_t& sound_control) {
    switch (field) {
        case sbar_field_audio_capture_name: {
            return sound_control.get_name();
        }
        case sbar_field_audio_capture_status: {
            if (! sound_control.has_capture_status()) {
                return RES_NEW_ERROR("This audio control does not have a "
                                     "capture status.\n\tname: "
                  + sound_control.get_name());
            }

            auto capture_status = sound_control.get_capture_status();
            if (capture_status.has_error()) {
                return RES_TRACE(capture_status.error());
            }

            std::string status;
            std::optional<bool> first;
            bool all_match = true;

            status += '(';

            status += make_audio_channel_status(
              capture_status->front_left, "fl", first, all_match);
            status += make_audio_channel_status(
              capture_status->front_center, "fc", first, all_match);
            status += make_audio_channel_status(
              capture_status->front_right, "fr", first, all_match);
            status += make_audio_channel_status(
              capture_status->side_left, "sl", first, all_match);
            status += make_audio_channel_status(
              capture_status->woofer, "w", first, all_match);
            status += make_audio_channel_status(
              capture_status->side_right, "sr", first, all_match);
            status += make_audio_channel_status(
              capture_status->rear_left, "rl", first, all_match);
            status += make_audio_channel_status(
              capture_status->rear_center, "rc", first, all_match);
            status += make_audio_channel_status(
              capture_status->rear_right, "rr", first, all_match);

            if (! first.has_value()) {
                return std::string{};
            }

            if (all_match) {
                return audio_channel_status_to_string(first.value());
            }

            status.back() = ')';

            return status;
        }
        case sbar_field_audio_capture_volume: {
            if (! sound_control.has_capture_volume()) {
                return RES_NEW_ERROR("This audio control does not have a "
                                     "capture volume.\n\tname: "
                  + sound_control.get_name());
            }

            auto capture_volume = sound_control.get_capture_volume();
            if (capture_volume.has_error()) {
                return RES_TRACE(capture_volume.error());
            }

            std::string status;
            std::optional<double> first;
            bool all_match = true;

            status += '(';

            status += make_audio_channel_volume(
              capture_volume->front_left, "fl", first, all_match);
            status += make_audio_channel_volume(
              capture_volume->front_center, "fc", first, all_match);
            status += make_audio_channel_volume(
              capture_volume->front_right, "fr", first, all_match);
            status += make_audio_channel_volume(
              capture_volume->side_left, "sl", first, all_match);
            status += make_audio_channel_volume(
              capture_volume->woofer, "w", first, all_match);
            status += make_audio_channel_volume(
              capture_volume->side_right, "sr", first, all_match);
            status += make_audio_channel_volume(
              capture_volume->rear_left, "rl", first, all_match);
            status += make_audio_channel_volume(
              capture_volume->rear_center, "rc", first, all_match);
            status += make_audio_channel_volume(
              capture_volume->rear_right, "rr", first, all_match);

            if (! first.has_value()) {
                return std::string{};
            }

            if (all_match) {
                return audio_channel_volume_to_string(first.value());
            }

            status.back() = ')';

            return status;
        }
        default:
            return RES_NEW_ERROR(
              "Invalid field value: " + std::to_string(field));
    }
}

[[nodiscard]] sbar_field_t status_field_assigner(char token) {
    switch (token) {
        case 'T':
            return sbar_field_time;
        case 'U':
            return sbar_field_uptime;
        case 'D':
            return sbar_field_disk;
        case 'S':
            return sbar_field_swap;
        case 'M':
            return sbar_field_memory;
        case 'W':
            return sbar_field_cpu;
        case 'w':
            return sbar_field_cpu_per_core;
        case 'H':
            return sbar_field_highest_temp;
        case 'L':
            return sbar_field_lowest_temp;
        case '1':
            return sbar_field_load_1;
        case '2':
            return sbar_field_load_5;
        case '3':
            return sbar_field_load_15;
        case 'b':
            return sbar_field_backlight;
        case 'B':
            return sbar_field_battery;
        case 'N':
            return sbar_field_network;
        case 'P':
            return sbar_field_audio_playback;
        case 'C':
            return sbar_field_audio_capture;
        case 'n':
            return sbar_field_username;
        case 'K':
            return sbar_field_kernel;
        case 'k':
            return sbar_field_outdated_kernel;
        default:
            return sbar_field_none;
    }
}

[[nodiscard]] res::optional_t<std::string> status_field_generator(
  sbar_field_t field, persistent_state_t& persistent_state) {
    switch (field) {
        case sbar_field_time: {
            std::time_t epoch_time = std::time(nullptr);
            std::tm* calendar_time = std::localtime(&epoch_time);

            // RFC 3339 format
            return sprintf("%i-%.2i-%.2i %.2i:%.2i:%.2i",
              calendar_time->tm_year + 1900,
              calendar_time->tm_mon + 1,
              calendar_time->tm_mday,
              calendar_time->tm_hour,
              calendar_time->tm_min,
              calendar_time->tm_sec);
        }
        case sbar_field_uptime: {
            if (! persistent_state.system_info.has_value()) {
                return RES_NEW_ERROR(
                  "Failed to get the uptime due to a "
                  "previous failure to get the system info.");
            }

            std::time_t epoch_uptime =
              persistent_state.system_info->uptime.count();
            std::tm* calendar_uptime = std::gmtime(&epoch_uptime);

            // non-standard format
            return sprintf("%i-%i %.2i:%.2i:%.2i",
              calendar_uptime->tm_year - 70,
              calendar_uptime->tm_yday,
              calendar_uptime->tm_hour,
              calendar_uptime->tm_min,
              calendar_uptime->tm_sec);
        }
        case sbar_field_disk: {
            auto disks = syst::get_disks();
            if (disks.has_error()) {
                return RES_TRACE(disks.error());
            }

            std::string status;

            for (const auto& disk : disks.value()) {
                if (persistent_state.ignore_zero_capacity_disks) {
                    auto disk_size = disk.get_size();
                    if (disk_size.has_error() || disk_size.value() == 0) {
                        continue;
                    }
                }

                status += make_given_status(persistent_state.disk_fmt,
                  persistent_state,
                  disk_field_assigner,
                  disk_field_generator,
                  disk);
            }

            return status;
        }
        case sbar_field_swap: {
            if (! persistent_state.system_info.has_value()) {
                return RES_NEW_ERROR(
                  "Failed to get the swap usage due to a "
                  "previous failure to get the system info.");
            }

            return sprintf(
              "%i", static_cast<int>(persistent_state.system_info->swap_usage));
        }
        case sbar_field_memory: {
            if (! persistent_state.system_info.has_value()) {
                return RES_NEW_ERROR(
                  "Failed to get the memory usage due to a "
                  "previous failure to get the system info.");
            }

            return sprintf(
              "%i", static_cast<int>(persistent_state.system_info->ram_usage));
        }
        case sbar_field_cpu: {
            auto usage = persistent_state.cpu_usage.get_total();
            if (usage.has_error()) {
                return RES_TRACE(usage.error());
            }

            return sprintf("%i", static_cast<int>(usage.value()));
        }
        case sbar_field_cpu_per_core: {
            auto cores = persistent_state.cpu_usage.get_per_core();
            if (cores.has_error()) {
                return RES_TRACE(cores.error());
            }

            if (cores->size() == 0) {
                return std::string{};
            }

            std::string status;

            for (auto usage : cores.value()) {
                auto status_part = sprintf("%i ", static_cast<int>(usage));
                if (status_part.has_error()) {
                    return RES_TRACE(status_part.error());
                }
                status += status_part.value();
            }

            status.pop_back();

            return status;
        }
        case sbar_field_highest_temp: {
            auto thermal_zones = syst::get_thermal_zones();
            if (thermal_zones.has_error()) {
                return RES_TRACE(thermal_zones.error());
            }

            std::optional<double> highest_temp;

            for (const auto& zone : thermal_zones.value()) {
                auto temp = zone.get_temperature();
                if (temp.has_error()) {
                    return RES_TRACE(temp.error());
                }

                if (! highest_temp.has_value()) {
                    highest_temp = temp.value();
                    continue;
                }

                if (temp.value() > highest_temp) {
                    highest_temp = temp.value();
                }
            }

            if (! highest_temp.has_value()) {
                return RES_NEW_ERROR(
                  "Failed to get the highest temperature measurement due to a "
                  "lack of any thermal measurements.");
            }

            return sprintf("%.0f", highest_temp.value());
        }
        case sbar_field_lowest_temp: {
            auto thermal_zones = syst::get_thermal_zones();
            if (thermal_zones.has_error()) {
                return RES_TRACE(thermal_zones.error());
            }

            std::optional<double> lowest_temp;

            for (const auto& zone : thermal_zones.value()) {
                auto temp = zone.get_temperature();
                if (temp.has_error()) {
                    return RES_TRACE(temp.error());
                }

                if (! lowest_temp.has_value()) {
                    lowest_temp = temp.value();
                    continue;
                }

                if (temp.value() < lowest_temp) {
                    lowest_temp = temp.value();
                }
            }

            if (! lowest_temp.has_value()) {
                return RES_NEW_ERROR(
                  "Failed to get the lowest temperature measurement due to a "
                  "lack of any thermal measurements.");
            }

            return sprintf("%.0f", lowest_temp.value());
        }
        case sbar_field_load_1: {
            if (! persistent_state.system_info.has_value()) {
                return RES_NEW_ERROR(
                  "Failed to get the 1 minute load average due to a "
                  "previous failure to get the system info.");
            }

            return sprintf("%.2f", persistent_state.system_info->load_1);
        }
        case sbar_field_load_5: {
            if (! persistent_state.system_info.has_value()) {
                return RES_NEW_ERROR(
                  "Failed to get the 5 minute load average due to a "
                  "previous failure to get the system info.");
            }

            return sprintf("%.2f", persistent_state.system_info->load_5);
        }
        case sbar_field_load_15: {
            if (! persistent_state.system_info.has_value()) {
                return RES_NEW_ERROR(
                  "Failed to get the 15 minute load average due to a "
                  "previous failure to get the system info.");
            }

            return sprintf("%.2f", persistent_state.system_info->load_15);
        }
        case sbar_field_backlight: {
            auto backlights = syst::get_backlights();
            if (backlights.has_error()) {
                return RES_TRACE(backlights.error());
            }

            std::string status;

            for (const auto& backlight : backlights.value()) {
                status += make_given_status(persistent_state.backlight_fmt,
                  persistent_state,
                  backlight_field_assigner,
                  backlight_field_generator,
                  backlight);
            }

            return status;
        }
        case sbar_field_battery: {
            auto batteries = syst::get_batteries();
            if (batteries.has_error()) {
                return RES_TRACE(batteries.error());
            }

            std::string status;

            for (const auto& battery : batteries.value()) {
                status += make_given_status(persistent_state.battery_fmt,
                  persistent_state,
                  battery_field_assigner,
                  battery_field_generator,
                  battery);
            }

            return status;
        }
        case sbar_field_network: {
            auto network_interfaces = syst::get_network_interfaces();
            if (network_interfaces.has_error()) {
                return RES_TRACE(network_interfaces.error());
            }

            std::string status;

            for (const auto& network_interface : network_interfaces.value()) {
                auto is_physical = network_interface.is_physical();
                if (is_physical.has_error()) {
                    std::cerr << is_physical.error() << std::endl;
                    continue;
                }
                if (! is_physical.value()) {
                    continue;
                }

                status += make_given_status(persistent_state.network_fmt,
                  persistent_state,
                  network_field_assigner,
                  network_field_generator,
                  network_interface);
            }

            return status;
        }
        case sbar_field_audio_playback: {
            if (! persistent_state.sound_mixer.has_value()) {
                return RES_NEW_ERROR(
                  "Failed to get audio playback info due to a previous failure "
                  "to get a sound mixer.");
            }

            auto controls = persistent_state.sound_mixer->get().get_controls();

            std::string status;

            for (const auto& control : controls) {
                if (! control.has_playback_status()
                  && ! control.has_playback_volume()) {
                    continue;
                }

                status += make_given_status(persistent_state.audio_playback_fmt,
                  persistent_state,
                  audio_playback_field_assigner,
                  audio_playback_field_generator,
                  control);
            }

            return status;
        }
        case sbar_field_audio_capture: {
            if (! persistent_state.sound_mixer.has_value()) {
                return RES_NEW_ERROR(
                  "Failed to get audio capture info due to a previous failure "
                  "to get a sound mixer.");
            }

            auto controls = persistent_state.sound_mixer->get().get_controls();

            std::string status;

            for (const auto& control : controls) {
                if (! control.has_capture_status()
                  && ! control.has_capture_volume()) {
                    continue;
                }

                status += make_given_status(persistent_state.audio_capture_fmt,
                  persistent_state,
                  audio_capture_field_assigner,
                  audio_capture_field_generator,
                  control);
            }

            return status;
        }
        case sbar_field_username: {
            auto username = syst::get_username();
            if (username.has_error()) {
                return RES_TRACE(username.error());
            }

            return username.value();
        }
        case sbar_field_kernel: {
            auto running_kernel = syst::get_running_kernel();
            if (running_kernel.has_error()) {
                return RES_TRACE(running_kernel.error());
            }

            return running_kernel.value();
        }
        case sbar_field_outdated_kernel: {
            auto running_kernel = syst::get_running_kernel();
            if (running_kernel.has_error()) {
                return RES_TRACE(running_kernel.error());
            }

            auto installed_kernels = syst::get_installed_kernels();
            if (installed_kernels.has_error()) {
                return RES_TRACE(installed_kernels.error());
            }

            for (const auto& version : installed_kernels.value()) {
                if (version == running_kernel.value()) {
                    return std::string{ "🟢" };
                }
            }

            return std::string{ "🔴" };
        }
        default:
            return RES_NEW_ERROR(
              "Invalid field value: " + std::to_string(field));
    }
}

int main(int argc, char** argv) {
    // Set signal handlers

    auto sigint_result = std::signal(SIGINT, signal_handler);
    if (sigint_result == SIG_ERR) {
        std::cerr << "Failed to set the signal handler for SIGINT."
                  << std::endl;
        return 1;
    }

    auto sigterm_result = std::signal(SIGTERM, signal_handler);
    if (sigterm_result == SIG_ERR) {
        std::cerr << "Failed to set the signal handler for SIGTERM."
                  << std::endl;
        return 1;
    }

    // Setup the argument parser

    argparse::ArgumentParser argparser{ "status_bar",
        sbar_get_runtime_version(),
        argparse::default_arguments::all,
        true };

    argparser.add_description("Status bar for dwm (https://dwm.suckless.org). "
                              "Customizable at runtime and updates instantly.");

    std::string default_fmt = "/P/C/N/B/b /W%c /H° /M%m /S%s |/D /T | /k /n";
    argparser.add_argument("-s", "--status")
      .nargs(1)
      .help(
        "custom status with the following interpreted sequences:\n"
        "    //    a literal /\n"
        "    /T    current time\n"
        "    /U    uptime\n"
        "    /D    disk and partition info | format with --disk-status\n"
        "    /S    swap usage percent\n"
        "    /M    memory usage percent\n"
        "    /W    CPU usage percent total\n"
        "    /w    CPU usage percent per core\n"
        "    /H    highest measured temperature (°C)\n"
        "    /L    lowest measured temperature (°C)\n"
        "    /1    1 minute load average\n"
        "    /2    5 minute load average\n"
        "    /3    15 minute load average\n"
        "    /b    backlight info | format with --backlight-status\n"
        "    /B    battery info | format with --battery-status\n"
        "    /N    network interface info | format with --network-status\n"
        "    /P    audio playback info | format with --sound-playback-status\n"
        "    /C    audio capture info | format with --sound-capture-status\n"
        "    /n    username\n"
        "    /K    running kernel name\n"
        "    /k    outdated kernel indicator\n    ")
      .default_value(default_fmt);

    std::string default_disk_fmt = "/P /R/E /F |";
    argparser.add_argument("-D", "--disk-status")
      .nargs(1)
      .help("custom disk status with the following interpreted sequences:\n"
            "    //    a literal /\n"
            "    /N    name\n"
            "    /O    rotational indicator\n"
            "    /R    read-only indicator\n"
            "    /E    removable without shutdown indicator\n"
            "    /C    size\n"
            "    /F    in-flight I/O operations\n"
            "    /P    partition info | format with --partition-status\n    ")
      .default_value(default_disk_fmt);

    std::string default_partition_fmt = " /M /U%";
    argparser.add_argument("-P", "--partition-status")
      .nargs(1)
      .help("custom disk partition status with the following interpreted "
            "sequences:\n"
            "    //    a literal /\n"
            "    /N    name\n"
            "    /R    read-only indicator\n"
            "    /M    mount path\n"
            "    /T    filesystem type\n"
            "    /C    size\n"
            "    /U    usage percent\n"
            "    /F    in-flight I/O operations\n    ")
      .default_value(default_partition_fmt);

    std::string default_backlight_fmt = " /L%l";
    argparser.add_argument("-b", "--backlight-status")
      .nargs(1)
      .help(
        "custom backlight status with the following interpreted sequences:\n"
        "    //    a literal /\n"
        "    /N    name\n"
        "    /L    brightness\n    ")
      .default_value(default_backlight_fmt);

    std::string default_battery_fmt = " /S /N /L% /T |";
    argparser.add_argument("-B", "--battery-status")
      .nargs(1)
      .help("custom battery status with the following interpreted sequences:\n"
            "    //    a literal /\n"
            "    /N    name\n"
            "    /S    status\n"
            "    /L    charge percent\n"
            "    /C    capacity percent\n"
            "    /I    current (A)\n"
            "    /P    power (W)\n"
            "    /T    time remaining\n    ")
      .default_value(default_battery_fmt);

    std::string default_network_interface_fmt = " /S /N |";
    argparser.add_argument("-N", "--network-status")
      .nargs(1)
      .help("custom network status with the following interpreted sequences:\n"
            "    //    a literal /\n"
            "    /N    name\n"
            "    /S    status\n"
            "    /R    packets down (received)\n"
            "    /T    packets up (transmitted)\n"
            "    /r    bytes down (received)\n"
            "    /t    bytes up (transmitted)\n    ")
      .default_value(default_network_interface_fmt);

    std::string default_audio_playback_fmt = " /S /V% |";
    argparser.add_argument("-P", "--audio-playback-status")
      .nargs(1)
      .help("custom audio playback status with the following interpreted "
            "sequences:\n"
            "    //    a literal /\n"
            "    /N    name\n"
            "    /S    status\n"
            "    /V    volume percent\n    ")
      .default_value(default_audio_playback_fmt);

    std::string default_audio_capture_fmt = " /S /V% |";
    argparser.add_argument("-C", "--audio-capture-status")
      .nargs(1)
      .help("custom audio capture status with the following interpreted "
            "sequences:\n"
            "    //    a literal /\n"
            "    /N    name\n"
            "    /S    status\n"
            "    /V    volume percent\n    ")
      .default_value(default_audio_capture_fmt);

    // Parse arguments
    try {
        argparser.parse_args(argc, argv);
    } catch (const std::exception& err) {
        std::cerr << err.what() << "\n\n";
        std::cerr << argparser;
        return 1;
    }

    persistent_state_t persistent_state;
    persistent_state.status_fmt = argparser.get<std::string>("--status");
    persistent_state.disk_fmt = argparser.get<std::string>("--disk-status");
    persistent_state.part_fmt =
      argparser.get<std::string>("--partition-status");
    persistent_state.backlight_fmt =
      argparser.get<std::string>("--backlight-status");
    persistent_state.battery_fmt =
      argparser.get<std::string>("--battery-status");
    persistent_state.network_fmt =
      argparser.get<std::string>("--network-status");
    persistent_state.audio_playback_fmt =
      argparser.get<std::string>("--audio-playback-status");
    persistent_state.audio_capture_fmt =
      argparser.get<std::string>("--audio-capture-status");
    persistent_state.ignore_zero_capacity_disks = true;

    auto channel = iipc::get_channel(sbar::channel);
    if (channel.has_error()) {
        std::cerr << channel.error() << std::endl;
        return 1;
    }

    auto root_window = sbar::get_root_window();
    if (root_window.has_error()) {
        std::cerr << root_window.error() << std::endl;
        return 1;
    }

    while (keep_running) {
        auto update_result = persistent_state.cpu_usage.update();
        if (update_result.failure()) {
            std::cerr << update_result.error() << std::endl;
        }

        auto system_state = syst::get_system_info();
        if (system_state.has_value()) {
            persistent_state.system_info = system_state.value();
        } else {
            persistent_state.system_info = std::nullopt;
            std::cerr << system_state.error() << std::endl;
        }

        auto sound_mixer = syst::get_sound_mixer();
        if (sound_mixer.has_value()) {
            persistent_state.sound_mixer = sound_mixer.value();
        } else {
            persistent_state.sound_mixer = std::nullopt;
            std::cerr << sound_mixer.error() << std::endl;
        }

        auto status = make_given_status(persistent_state.status_fmt,
          persistent_state,
          status_field_assigner,
          status_field_generator);
        persistent_state.fields_to_update = sbar_field_all;

        auto result = root_window->set_title(status);
        if (result.failure()) {
            std::cerr << result.error() << std::endl;
        }

        auto poll_result = channel->poll(std::chrono::seconds(1));
        if (poll_result.has_error()) {
            std::cerr << poll_result.error() << std::endl;
            continue;
        }
        if (! poll_result.value()) {
            continue;
        }

        auto receive_result = channel->receive();
        if (receive_result.has_value()) {
            persistent_state.fields_to_update =
              static_cast<sbar_field_t>(std::stoull(receive_result.value()));
        } else {
            std::cerr << receive_result.error() << std::endl;
        }
    }

    // Reset the title of the root window before exiting.
    auto result = root_window->set_title("");
    if (result.failure()) {
        std::cerr << result.error() << std::endl;
        return 1;
    }

    return 0;
}
