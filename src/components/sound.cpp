// Standard includes
#include <string>

// Local includes
#include "../status.hpp"

namespace sbar {

std::string Fields::get_volume_status() {
    if (! this->sound_mixer.good() && ! this->sound_mixer.init()) {
        return sbar::error_str;
    }
    return this->sound_mixer.get_playback_status();
}

std::string Fields::get_volume_percent() {
    if (! this->sound_mixer.good() && ! this->sound_mixer.init()) {
        return sbar::error_str;
    }
    return this->sound_mixer.get_playback_volume();
}

std::string Fields::get_capture_status() {
    if (! this->sound_mixer.good() && ! this->sound_mixer.init()) {
        return sbar::error_str;
    }
    return this->sound_mixer.get_capture_status();
}

std::string Fields::get_capture_percent() {
    if (! this->sound_mixer.good() && ! this->sound_mixer.init()) {
        return sbar::error_str;
    }
    return this->sound_mixer.get_capture_volume();
}

} // namespace sbar
