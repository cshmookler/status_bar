/*****************************************************************************/
/*  Copyright (c) 2025 Caden Shmookler                                       */
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

#ifndef SBAR_NOTIFY_H
#define SBAR_NOTIFY_H

#ifdef __cplusplus
extern "C" {
#endif

// clang-format off
const unsigned long long sbar_line_start = __LINE__ + 3;
enum sbar_field_t : unsigned long long {
    sbar_field_none = 0ULL,
    sbar_field_time = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_uptime = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_disk = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_disk_name = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_disk_rotational = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_disk_read_only = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_disk_removable = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_disk_size = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_disk_in_flight = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_part = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_part_name = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_part_read_only = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_part_mount = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_part_filesystem = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_part_size = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_part_usage = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_part_in_flight = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_swap = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_memory = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_cpu = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_cpu_per_core = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_highest_temp = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_lowest_temp = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_load_1 = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_load_5 = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_load_15 = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_backlight = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_backlight_name = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_backlight_brightness = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_battery = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_battery_name = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_battery_status = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_battery_charge = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_battery_capacity = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_battery_current = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_battery_power = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_battery_time = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_network = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_network_name = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_network_status = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_network_packets_down = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_network_packets_up = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_network_bytes_down = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_network_bytes_up = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_audio_playback = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_audio_playback_name = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_audio_playback_status = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_audio_playback_volume = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_audio_capture = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_audio_capture_name = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_audio_capture_status = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_audio_capture_volume = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_username = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_kernel = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_outdated_kernel = 1ULL << (__LINE__ - sbar_line_start),
    sbar_field_all = (1ULL << (__LINE__ - sbar_line_start)) - 1ULL,
};
const unsigned long long sbar_line_end = __LINE__ - 3;
const unsigned long long sbar_total_fields = sbar_line_end - sbar_line_start + 1;
// clang-format on

/**
 * @brief Notifies the status bar that certain specified fields must be
 * updated immediately.
 *
 * @param[in] fields - The fields to be updated.
 * @return 0 if this notification was successfully dispatched and 1
 * otherwise.
 */
int sbar_notify(sbar_field_t fields);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SBAR_NOTIFY_H
