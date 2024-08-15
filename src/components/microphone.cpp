// Standard includes
#include <cstddef>
#include <filesystem>
#include <string>
#include <string_view>

// Local includes
#include "../utils.hpp"

namespace sbar {

std::string get_microphone_status() {
    const char* const asound_path = "/proc/asound/";
    const char* const card_prefix = "card";
    const char* const device_prefix = "pcm";
    const char* const device_postfix = "c"; // 'c' for capture
    const char* const status_filename = "status";

    size_t closed_device_count = 0;

    for (const std::filesystem::directory_entry& card :
      std::filesystem::directory_iterator(asound_path)) {
        if (! card.is_directory()) {
            continue;
        }

        std::string card_name = card.path().filename().string();
        std::string_view card_name_raw{ card_name };

        if (! remove_prefix(card_name_raw, card_prefix)) {
            continue;
        }

        for (const std::filesystem::directory_entry& device :
          std::filesystem::directory_iterator(card)) {
            if (! device.is_directory()) {
                continue;
            }

            std::string device_name = device.path().filename().string();
            std::string_view device_name_raw{ device_name };

            if (! (remove_prefix(device_name_raw, device_prefix)
                  && remove_postfix(device_name_raw, device_postfix))) {
                continue;
            }

            for (const std::filesystem::directory_entry& sub_device :
              std::filesystem::directory_iterator(device)) {
                if (! sub_device.is_directory()) {
                    continue;
                }

                std::string status = get_first_line(
                  sub_device / std::filesystem::path(status_filename));

                if (status != "closed") {
                    return "🟢";
                }

                closed_device_count++;
            }
        }
    }

    if (closed_device_count == 0) {
        return "❌";
    }

    return "🔴";
}

} // namespace sbar
