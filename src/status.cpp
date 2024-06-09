// Standard includes
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

// External includes
#include <sys/sysinfo.h>

// Local includes
#include "constants.hpp"
#include "cpu.hpp"
#include "status.hpp"

namespace status_bar {

template<typename... Args>
std::string sprintf(const char* format, Args... args) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, hicpp-vararg)
    int size = std::snprintf(nullptr, 0, format, args...);
    std::string buffer(size, '\0');
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, hicpp-vararg)
    if (std::sprintf(buffer.data(), format, args...) < 0) {
        return error_str;
    }
    return buffer;
}

std::string time() {
    std::time_t epoch_time = std::time(nullptr);
    std::tm* calendar_time = std::localtime(&epoch_time);

    // RFC 3339 format
    return sprintf(
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
        return error_str;
    }

    std::time_t epoch_uptime = system_info.uptime;
    std::tm* calendar_uptime = std::gmtime(&epoch_uptime);

    // non-standard format
    return sprintf(
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

    return sprintf("%.0f", (used / total) * 1e2);
}

std::string swap_percent() {
    struct sysinfo system_info {};
    if (sysinfo(&system_info) != 0) {
        return error_str;
    }

    auto total = static_cast<double>(system_info.totalswap);
    auto used =
            static_cast<double>(system_info.totalswap - system_info.freeswap);

    return sprintf("%.0f", (used / total) * 1e2);
}

std::string memory_percent() {
    struct sysinfo system_info {};
    if (sysinfo(&system_info) != 0) {
        return error_str;
    }

    auto total = static_cast<double>(system_info.totalram);
    auto used = static_cast<double>(
            system_info.totalram - system_info.freeram - system_info.bufferram
            - system_info.sharedram);

    return sprintf("%.0f", (used / total) * 1e2);
}

std::string cpu_percent(std::unique_ptr<cpu>& cpu_stat) {
    if (cpu_stat == nullptr) {
        cpu_stat = std::make_unique<cpu>();
        return standby_str;
    }

    try {
        std::vector<cpu::index> idle_components{ cpu::index::idle };

        auto prev_total = cpu_stat->get_total();
        auto prev_idle = cpu_stat->get_total(idle_components);
        auto prev_work = prev_total - prev_idle;

        cpu_stat = std::make_unique<cpu>();

        auto new_total = cpu_stat->get_total();
        auto new_idle = cpu_stat->get_total(idle_components);
        auto new_work = new_total - new_idle;

        auto total_diff = static_cast<double>(new_total - prev_total);
        auto work_diff = static_cast<double>(new_work - prev_work);

        return sprintf("%.0f", (work_diff / total_diff) * 1e2);
    }
    catch (const invalid_proc_stat& err) {
        std::cerr << err.what() << '\n';
        return err.status();
    }
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

} // namespace status_bar
