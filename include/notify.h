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

enum sbar_field_t : unsigned long long {
    sbar_field_none = 0ULL,
    sbar_field_time = 1ULL,
    sbar_field_uptime = sbar_field_time << 1,
    sbar_field_disk = sbar_field_uptime << 1,
    sbar_field_disk_name = sbar_field_disk << 1,
    sbar_field_disk_rotational = sbar_field_disk_name << 1,
    sbar_field_disk_read_only = sbar_field_disk_rotational << 1,
    sbar_field_disk_removable = sbar_field_disk_read_only << 1,
    sbar_field_disk_size = sbar_field_disk_removable << 1,
    sbar_field_disk_in_flight = sbar_field_disk_size << 1,
    sbar_field_part = sbar_field_disk_in_flight << 1,
    sbar_field_part_name = sbar_field_part << 1,
    sbar_field_part_read_only = sbar_field_part_name << 1,
    sbar_field_part_mount = sbar_field_part_read_only << 1,
    sbar_field_part_filesystem = sbar_field_part_mount << 1,
    sbar_field_part_size = sbar_field_part_filesystem << 1,
    sbar_field_part_usage = sbar_field_part_size << 1,
    sbar_field_part_in_flight = sbar_field_part_usage << 1,
    sbar_field_swap = sbar_field_part_in_flight << 1,
    sbar_field_memory = sbar_field_swap << 1,
    sbar_field_cpu = sbar_field_memory << 1,
    sbar_field_cpu_per_core = sbar_field_cpu << 1,
    sbar_field_highest_temp = sbar_field_cpu_per_core << 1,
    sbar_field_lowest_temp = sbar_field_highest_temp << 1,
    sbar_field_load_1 = sbar_field_lowest_temp << 1,
    sbar_field_load_5 = sbar_field_load_1 << 1,
    sbar_field_load_15 = sbar_field_load_5 << 1,
    sbar_field_backlight = sbar_field_load_15 << 1,
    sbar_field_backlight_name = sbar_field_backlight << 1,
    sbar_field_backlight_brightness = sbar_field_backlight_name << 1,
    sbar_field_battery = sbar_field_backlight_brightness << 1,
    sbar_field_battery_name = sbar_field_battery << 1,
    sbar_field_battery_status = sbar_field_battery_name << 1,
    sbar_field_battery_charge = sbar_field_battery_status << 1,
    sbar_field_battery_capacity = sbar_field_battery_charge << 1,
    sbar_field_battery_current = sbar_field_battery_capacity << 1,
    sbar_field_battery_power = sbar_field_battery_current << 1,
    sbar_field_battery_time = sbar_field_battery_power << 1,
    sbar_field_network = sbar_field_battery_time << 1,
    sbar_field_network_name = sbar_field_network << 1,
    sbar_field_network_status = sbar_field_network_name << 1,
    sbar_field_network_packets_down = sbar_field_network_status << 1,
    sbar_field_network_packets_up = sbar_field_network_packets_down << 1,
    sbar_field_network_bytes_down = sbar_field_network_packets_up << 1,
    sbar_field_network_bytes_up = sbar_field_network_bytes_down << 1,
    sbar_field_audio_playback = sbar_field_network_bytes_up << 1,
    sbar_field_audio_playback_name = sbar_field_audio_playback << 1,
    sbar_field_audio_playback_status = sbar_field_audio_playback_name << 1,
    sbar_field_audio_playback_volume = sbar_field_audio_playback_status << 1,
    sbar_field_audio_capture = sbar_field_audio_playback_volume << 1,
    sbar_field_audio_capture_name = sbar_field_audio_capture << 1,
    sbar_field_audio_capture_status = sbar_field_audio_capture_name << 1,
    sbar_field_audio_capture_volume = sbar_field_audio_capture_status << 1,
    sbar_field_username = sbar_field_audio_capture_volume << 1,
    sbar_field_kernel = sbar_field_username << 1,
    sbar_field_outdated_kernel = sbar_field_kernel << 1,
    sbar_field_all = (sbar_field_outdated_kernel << 1) - 1ULL,
};
typedef enum sbar_field_t sbar_field_t;

/**
 * @brief The total number of fields to select from.
 */
const unsigned long long sbar_total_fields =
  __builtin_ctzll(sbar_field_all + 1ULL);

enum sbar_top_field_t : unsigned long long {
    sbar_top_field_none = sbar_field_none,
    sbar_top_field_time = sbar_field_time,
    sbar_top_field_uptime = sbar_field_uptime,
    sbar_top_field_disk = sbar_field_disk | sbar_field_disk_name
      | sbar_field_disk_rotational | sbar_field_disk_read_only
      | sbar_field_disk_removable | sbar_field_disk_size
      | sbar_field_disk_in_flight | sbar_field_part | sbar_field_part_name
      | sbar_field_part_read_only | sbar_field_part_mount
      | sbar_field_part_filesystem | sbar_field_part_size
      | sbar_field_part_usage | sbar_field_part_in_flight,
    sbar_top_field_swap = sbar_field_swap,
    sbar_top_field_memory = sbar_field_memory,
    sbar_top_field_cpu = sbar_field_cpu,
    sbar_top_field_cpu_per_core = sbar_field_cpu_per_core,
    sbar_top_field_highest_temp = sbar_field_highest_temp,
    sbar_top_field_lowest_temp = sbar_field_lowest_temp,
    sbar_top_field_load_1 = sbar_field_load_1,
    sbar_top_field_load_5 = sbar_field_load_5,
    sbar_top_field_load_15 = sbar_field_load_15,
    sbar_top_field_backlight = sbar_field_backlight | sbar_field_backlight_name
      | sbar_field_backlight_brightness,
    sbar_top_field_battery = sbar_field_battery | sbar_field_battery_name
      | sbar_field_battery_status | sbar_field_battery_charge
      | sbar_field_battery_capacity | sbar_field_battery_current
      | sbar_field_battery_power | sbar_field_battery_time,
    sbar_top_field_network = sbar_field_network | sbar_field_network_name
      | sbar_field_network_status | sbar_field_network_packets_down
      | sbar_field_network_packets_up | sbar_field_network_bytes_down
      | sbar_field_network_bytes_up,
    sbar_top_field_audio_playback = sbar_field_audio_playback
      | sbar_field_audio_playback_name | sbar_field_audio_playback_status
      | sbar_field_audio_playback_volume,
    sbar_top_field_audio_capture = sbar_field_audio_capture
      | sbar_field_audio_capture_name | sbar_field_audio_capture_status
      | sbar_field_audio_capture_volume,
    sbar_top_field_username = sbar_field_username,
    sbar_top_field_kernel = sbar_field_kernel,
    sbar_top_field_outdated_kernel = sbar_field_outdated_kernel,
    sbar_top_field_all = sbar_field_all,
};
typedef enum sbar_top_field_t sbar_top_field_t;

/**
 * @brief Notifies the status bar that certain specified fields must be
 * updated immediately.
 *
 * @param[in] fields - The fields to be updated.
 * @return 0 if this notification was successfully dispatched and 1
 * otherwise.
 */
int sbar_notify(sbar_top_field_t fields);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SBAR_NOTIFY_H
