// Standard includes
#include <string>

// Local includes
#include "../status.hpp"

namespace sbar {

std::optional<Sound_mixer> Optional_sound_mixer::constructor_() {
    std::optional<Sound_mixer> sound_mixer = std::make_optional<Sound_mixer>();
    if (! sound_mixer->good()) {
        return std::nullopt;
    }
    return sound_mixer;
}

std::string get_volume_state(const Sound_mixer& mixer) {
    return mixer.get_playback_state();
}

std::string get_volume_perc(const Sound_mixer& mixer) {
    return mixer.get_playback_volume();
}

std::string get_capture_state(const Sound_mixer& mixer) {
    return mixer.get_capture_state();
}

std::string get_capture_perc(const Sound_mixer& mixer) {
    return mixer.get_capture_volume();
}

} // namespace sbar
