// Standard includes
#include <filesystem>
#include <string>

// External includes
#include <sys/sysinfo.h>

// Local includes
#include "../status.hpp"
#include "../utils.hpp"

namespace sbar {

std::string get_disk_percent() {
    std::filesystem::space_info root_dir =
      std::filesystem::space(std::filesystem::current_path().root_path());

    auto total = static_cast<double>(root_dir.capacity);
    auto used = static_cast<double>(root_dir.capacity - root_dir.available);

    return sprintf("%.0f", (used / total) * 1e2);
}

std::string get_swap_percent(const System& system) {
    auto total = static_cast<double>(system->totalswap);
    auto used = static_cast<double>(system->totalswap - system->freeswap);

    return sprintf("%.0f", (used / total) * 1e2);
}

std::string get_memory_percent(const System& system) {
    auto total = static_cast<double>(system->totalram);
    auto used = static_cast<double>(system->totalram - system->freeram
      - system->bufferram - system->sharedram);

    return sprintf("%.0f", (used / total) * 1e2);
}

} // namespace sbar
