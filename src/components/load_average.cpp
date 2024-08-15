// Standard includes
#include <string>

// External includes
#include <sys/sysinfo.h>

// Local includes
#include "../status.hpp"
#include "../utils.hpp"

namespace sbar {

float get_load_average(unsigned long load) {
    return static_cast<float>(load) / static_cast<float>(1U << SI_LOAD_SHIFT);
}

std::string get_one_minute_load_average(System& system) {
    if (! system.good() && ! system.init()) {
        return sbar::error_str;
    }
    return sprintf("%.1f", get_load_average(system->loads[0]));
}

std::string get_five_minute_load_average(System& system) {
    if (! system.good() && ! system.init()) {
        return sbar::error_str;
    }
    return sprintf("%.1f", get_load_average(system->loads[1]));
}

std::string get_fifteen_minute_load_average(System& system) {
    if (! system.good() && ! system.init()) {
        return sbar::error_str;
    }
    return sprintf("%.1f", get_load_average(system->loads[2]));
}

} // namespace sbar
