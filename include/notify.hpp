#pragma once

/**
 * @file notify.hpp
 * @author Caden Shmookler (cshmookler@gmail.com)
 * @brief Utilities for notifying the status bar that it is out-of-date.
 * @date 2024-08-08
 */

// Standard includes
#include <cstddef>
#include <limits>
#include <optional>

namespace sbar {

constexpr const char* notify_path = "/tmp/status_bar";

[[nodiscard]] inline constexpr size_t bit(size_t index) {
    return static_cast<size_t>(1) << index;
}

[[nodiscard]] constexpr size_t index(size_t bit) {
    if (bit == 0) {
        return std::numeric_limits<size_t>::max();
    }

    size_t value = 0;
    while ((bit & 1) != 1) {
        bit >>= 1;
        value++;
    }

    return value;
}

// clang-format off
namespace {
constexpr size_t field_first_line = __LINE__ + 4;
} // namespace
// Do NOT change the spacing here!
enum class field : size_t {
    time = bit(__LINE__ - field_first_line),
    uptime = bit(__LINE__ - field_first_line),
    disk = bit(__LINE__ - field_first_line),
    swap = bit(__LINE__ - field_first_line),
    memory = bit(__LINE__ - field_first_line),
    cpu = bit(__LINE__ - field_first_line),
    cpu_temp = bit(__LINE__ - field_first_line),
    load_1 = bit(__LINE__ - field_first_line),
    load_5 = bit(__LINE__ - field_first_line),
    load_15 = bit(__LINE__ - field_first_line),
    battery_status = bit(__LINE__ - field_first_line),
    battery_device = bit(__LINE__ - field_first_line),
    battery = bit(__LINE__ - field_first_line),
    battery_time = bit(__LINE__ - field_first_line),
    backlight = bit(__LINE__ - field_first_line),
    network_status = bit(__LINE__ - field_first_line),
    network_device = bit(__LINE__ - field_first_line),
    network_ssid = bit(__LINE__ - field_first_line),
    network_strength = bit(__LINE__ - field_first_line),
    network_upload = bit(__LINE__ - field_first_line),
    network_download = bit(__LINE__ - field_first_line),
    volume_status = bit(__LINE__ - field_first_line),
    volume = bit(__LINE__ - field_first_line),
    capture_status = bit(__LINE__ - field_first_line),
    capture = bit(__LINE__ - field_first_line),
    microphone = bit(__LINE__ - field_first_line),
    camera = bit(__LINE__ - field_first_line),
    user = bit(__LINE__ - field_first_line),
    kernel_status = bit(__LINE__ - field_first_line),
};
// Do NOT change the spacing here!
namespace {
constexpr size_t field_last_line = __LINE__ - 4;
} // namespace
// clang-format on
//
constexpr size_t field_count = field_last_line - field_first_line + 1;

constexpr field field_none = static_cast<field>(0);
constexpr field field_all = static_cast<field>(bit(field_count) - 1);

inline constexpr field operator|(field lhs, size_t rhs) {
    return static_cast<field>(static_cast<size_t>(lhs) | rhs);
}
inline constexpr field operator|(size_t lhs, field rhs) {
    return operator|(rhs, lhs);
}
inline constexpr field operator|(field lhs, field rhs) {
    return operator|(lhs, static_cast<size_t>(rhs));
}
inline constexpr field operator&(field lhs, size_t rhs) {
    return static_cast<field>(static_cast<size_t>(lhs) & rhs);
}
inline constexpr field operator&(size_t lhs, field rhs) {
    return operator&(rhs, lhs);
}
inline constexpr field operator&(field lhs, field rhs) {
    return operator&(lhs, static_cast<size_t>(rhs));
}

[[nodiscard]] bool notify(field fields);

[[nodiscard]] std::optional<field> get_notification();

} // namespace sbar
