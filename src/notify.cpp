// Standard includes
#include <bitset>
#include <cstddef>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>

// Local includes
#include "../include/notify.hpp"

namespace sbar {

bool notify(field fields) {
    std::ofstream file{ notify_path };
    if (! file.good()) {
        return false;
    }

    std::string fields_str =
      std::bitset<field_count>(static_cast<size_t>(fields)).to_string();
    file.write(
      fields_str.data(), static_cast<std::streamsize>(fields_str.size()));

    return true;
}

std::optional<field> get_notification() {
    std::fstream file{ notify_path, std::ios_base::in | std::ios_base::ate };
    std::string fields_str(file.tellg(), '\0');
    file.seekg(0);
    file.read(fields_str.data(), fields_str.size());
    file.close();

    try {
        return { static_cast<field>(std::stoull(fields_str, 0, 2)) };
    } catch (const std::invalid_argument& error) {
        return std::nullopt;
    }
}

} // namespace sbar
