// Standard includes
#include <filesystem>
#include <string>

// External includes
#include <sys/sysinfo.h>

// Local includes
#include "../status.hpp"
#include "../utils.hpp"

namespace sbar {

std::string Fields::get_disk_percent() {
    std::filesystem::space_info root_dir =
      std::filesystem::space(std::filesystem::current_path().root_path());

    auto total = static_cast<double>(root_dir.capacity);
    auto used = static_cast<double>(root_dir.capacity - root_dir.available);

    return sprintf("%.0f", (used / total) * 1e2);
}

std::string Fields::get_swap_percent() {
    if (! this->system.good() && ! this->system.init()) {
        return sbar::error_str;
    }

    auto total = static_cast<double>(this->system->totalswap);
    auto used =
      static_cast<double>(this->system->totalswap - this->system->freeswap);

    return sprintf("%.0f", (used / total) * 1e2);
}

std::string Fields::get_memory_percent() {
    if (! this->system.good() && ! this->system.init()) {
        return sbar::error_str;
    }

    auto total = static_cast<double>(this->system->totalram);
    auto used =
      static_cast<double>(this->system->totalram - this->system->freeram
        - this->system->bufferram - this->system->sharedram);

    return sprintf("%.0f", (used / total) * 1e2);
}

} // namespace sbar
