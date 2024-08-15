// Standard includes
#include <string>

// Local includes
#include "../status.hpp"

namespace sbar {

std::string get_volume_status(Sound_mixer& mixer) {
    if (! mixer.good() && ! mixer.init()) {
        return sbar::error_str;
    }
    return mixer.get_playback_status();
}

std::string get_volume_perc(Sound_mixer& mixer) {
    if (! mixer.good() && ! mixer.init()) {
        return sbar::error_str;
    }
    return mixer.get_playback_volume();
}

std::string get_capture_status(Sound_mixer& mixer) {
    if (! mixer.good() && ! mixer.init()) {
        return sbar::error_str;
    }
    return mixer.get_capture_status();
}

std::string get_capture_perc(Sound_mixer& mixer) {
    if (! mixer.good() && ! mixer.init()) {
        return sbar::error_str;
    }
    return mixer.get_capture_volume();
}

} // namespace sbar
