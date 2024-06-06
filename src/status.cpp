// Standard includes
#include <ctime>
#include <format>

// External includes
// #include <sys/statvfs.h>

// Local includes
#include "status.hpp"

namespace status_bar {

std::string time() {
    std::time_t epoch_time = std::time(nullptr);
    std::tm* calendar_time = std::localtime(&epoch_time);

    // Format the current time in RFC 3339 format
    return std::format(
            "{}-{:0>2}-{:0>2} {:0>2}:{:0>2}:{:0>2}",
            calendar_time->tm_year + 1900, // NOLINT(readability-magic-numbers)
            calendar_time->tm_mon + 1,
            calendar_time->tm_mday,
            calendar_time->tm_hour,
            calendar_time->tm_min,
            calendar_time->tm_sec);
}

std::string disk_percent() {
    return "";
}

std::string memory_percent() {
    return "";
}

std::string cpu_percent() {
    return "";
}

std::string battery_state() {
    return "";
}

std::string battery_perc() {
    return "";
}

std::string backlight_perc() {
    return "";
}

std::string network_ssid() {
    return "";
}

std::string wifi_perc() {
    return "";
}

std::string bluetooth_devices() {
    return "";
}

std::string volume_status() {
    return "";
}

std::string volume_perc() {
    return "";
}

std::string microphone_state() {
    return "";
}

std::string camera_state() {
    return "";
}

// std::string time() {
//     std::string str(100, '\0'); // NOLINT(readability-magic-numbers)
//     std::time_t now = std::time(nullptr);
//     str.resize(std::strftime(
//             str.data(),
//             str.size(),
//             "%Z %F %a %T",
//             std::localtime(&now))); // NOLINT(concurrency-mt-unsafe)
//     return str;
// }

// std::string disk_percent() {
//     struct statvfs root_fs {};
//     statvfs("/", &root_fs);
//     uint disk_percent =
//             // NOLINTNEXTLINE(readability-magic-numbers)
//             100UL - ((root_fs.f_bavail * 100UL) / root_fs.f_blocks);
//     return std::to_string(disk_percent);
// }

// std::string memory_percent() {
//     uintmax_t total = 0;
//     uintmax_t available = 0;

//     // TODO: Write general function for extracting data from commands and
//     files

//     int identifiers_found =
//             pscanf("/proc/meminfo",
//                    "MemTotal: %ju kB\n"
//                    "MemAvailable: %ju kB\n",
//                    &total,
//                    &available);
//     if (identifiers_found != 2) // NOLINT(readability-magic-numbers)
//         return "n/a";

//     int memory_percent = (available * 100UL) / total;
//     return std::to_string(memory_percent);
// }

} // namespace status_bar
