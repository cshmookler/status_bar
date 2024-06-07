// Standard includes
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <filesystem>

// External includes
#include <sys/sysinfo.h>

// Local includes
#include "status.hpp"

namespace status_bar {

namespace internal {

template<typename... Args>
std::string sprintf(const char* format, Args... args) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, hicpp-vararg)
    int size = std::snprintf(nullptr, 0, format, args...);
    std::string buffer(size, '\0');
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, hicpp-vararg)
    if (std::sprintf(buffer.data(), format, args...) < 0) {
        return "n/a";
    }
    return buffer;
}

} // namespace internal

std::string time() {
    std::time_t epoch_time = std::time(nullptr);
    std::tm* calendar_time = std::localtime(&epoch_time);

    // RFC 3339 format
    return internal::sprintf(
            "%i-%.2i-%.2i %.2i:%.2i:%.2i",
            calendar_time->tm_year + 1900, // NOLINT(readability-magic-numbers)
            calendar_time->tm_mon + 1,
            calendar_time->tm_mday,
            calendar_time->tm_hour,
            calendar_time->tm_min,
            calendar_time->tm_sec);
}

[[nodiscard]] std::string uptime() {
    struct sysinfo system_info {};
    if (sysinfo(&system_info) != 0) {
        return "n/a";
    }

    std::time_t epoch_uptime = system_info.uptime;
    std::tm* calendar_uptime = std::gmtime(&epoch_uptime);

    // non-standard format
    return internal::sprintf(
            "%i-%.3i %.2i:%.2i:%.2i",
            calendar_uptime->tm_year - 70, // NOLINT(readability-magic-numbers)
            calendar_uptime->tm_yday,
            calendar_uptime->tm_hour,
            calendar_uptime->tm_min,
            calendar_uptime->tm_sec);
}

std::string disk_percent() {
    std::filesystem::space_info root_dir =
            std::filesystem::space(std::filesystem::current_path().root_path());

    auto total = static_cast<double>(root_dir.capacity);
    auto used = static_cast<double>(root_dir.capacity - root_dir.available);

    return internal::sprintf("%.0f", (used / total) * 1e2);
}

std::string swap_percent() {
    struct sysinfo system_info {};
    if (sysinfo(&system_info) != 0) {
        return "n/a";
    }

    auto total = static_cast<double>(system_info.totalswap);
    auto used =
            static_cast<double>(system_info.totalswap - system_info.freeswap);

    return internal::sprintf("%.0f", (used / total) * 1e2);
}

std::string memory_percent() {
    struct sysinfo system_info {};
    if (sysinfo(&system_info) != 0) {
        return "n/a";
    }

    auto total = static_cast<double>(system_info.totalram);
    auto used = static_cast<double>(
            system_info.totalram - system_info.freeram - system_info.bufferram
            - system_info.sharedram);

    return internal::sprintf("%.0f", (used / total) * 1e2);
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
