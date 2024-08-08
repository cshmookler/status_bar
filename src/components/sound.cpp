// Standard includes
#include <string>

// Local includes
#include "../status.hpp"

namespace sbar {

std::string get_volume_status(const Sound_mixer& mixer) {
    return mixer.get_playback_status();
}

std::string get_volume_perc(const Sound_mixer& mixer) {
    return mixer.get_playback_volume();
}

std::string get_capture_status(const Sound_mixer& mixer) {
    return mixer.get_capture_status();
}

std::string get_capture_perc(const Sound_mixer& mixer) {
    return mixer.get_capture_volume();
}

} // namespace sbar
