#pragma once

/*****************************************************************************/
/*  Copyright (c) 2024 Caden Shmookler                                       */
/*                                                                           */
/*  This software is provided 'as-is', without any express or implied        */
/*  warranty. In no event will the authors be held liable for any damages    */
/*  arising from the use of this software.                                   */
/*                                                                           */
/*  Permission is granted to anyone to use this software for any purpose,    */
/*  including commercial applications, and to alter it and redistribute it   */
/*  freely, subject to the following restrictions:                           */
/*                                                                           */
/*  1. The origin of this software must not be misrepresented; you must not  */
/*     claim that you wrote the original software. If you use this software  */
/*     in a product, an acknowledgment in the product documentation would    */
/*     be appreciated but is not required.                                   */
/*  2. Altered source versions must be plainly marked as such, and must not  */
/*     be misrepresented as being the original software.                     */
/*  3. This notice may not be removed or altered from any source             */
/*     distribution.                                                         */
/*****************************************************************************/

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

/**
 * @brief Returns a number with a single positive bit cooresponding to the given
 * index.
 *
 * @param[in] index - The index of the lone positive bit.
 */
[[nodiscard]] inline constexpr size_t bit(size_t index) {
    return static_cast<size_t>(1) << index;
}

/**
 * @brief Returns the number of leading zeros before the first non-zero bit in
 * the given number. The numerical maximum is returned if the given number is
 * zero.
 *
 * @param[in] bit - The number containing at least one positive bit.
 */
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

/**
 * @brief Notifies the status bar that certain specified fields must be updated
 * immediately.
 *
 * @param[in] fields - The fields to be updated.
 * @return true if this notification was successfully dispatched and false
 * otherwise.
 */
bool notify(field fields);

/**
 * @brief Retrieves the most recent status bar notification.
 *
 * @return the fields to be updated or std::nullopt if the notification file
 * could not be read.
 */
[[nodiscard]] std::optional<field> get_notification();

} // namespace sbar
