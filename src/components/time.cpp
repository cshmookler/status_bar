// Standard includes
#include <ctime>
#include <string>

// External includes
#include <sys/sysinfo.h>

// Local includes
#include "../status.hpp"
#include "../utils.hpp"

namespace sbar {

std::string get_time() {
    std::time_t epoch_time = std::time(nullptr);
    std::tm* calendar_time = std::localtime(&epoch_time);

    // RFC 3339 format
    return sprintf("%i-%.2i-%.2i %.2i:%.2i:%.2i",
      calendar_time->tm_year + 1900, // NOLINT(readability-magic-numbers)
      calendar_time->tm_mon + 1,
      calendar_time->tm_mday,
      calendar_time->tm_hour,
      calendar_time->tm_min,
      calendar_time->tm_sec);
}

std::string get_uptime(const System& system_info) {
    std::time_t epoch_uptime = system_info->uptime;
    std::tm* calendar_uptime = std::gmtime(&epoch_uptime);

    // non-standard format
    return sprintf("%i-%.3i %.2i:%.2i:%.2i",
      calendar_uptime->tm_year - 70, // NOLINT(readability-magic-numbers)
      calendar_uptime->tm_yday,
      calendar_uptime->tm_hour,
      calendar_uptime->tm_min,
      calendar_uptime->tm_sec);
}

} // namespace sbar
