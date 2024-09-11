// External includes
#include <X11/Xlib.h>

// Local includes
#include "root_window.hpp"

namespace sbar {

Root_window::Root_window() : display_(XOpenDisplay(nullptr)) {
}

Root_window::~Root_window() {
    if (this->good()) {
        XCloseDisplay(static_cast<Display*>(this->display_));
    }
}

[[nodiscard]] bool Root_window::good() const {
    return this->display_ != nullptr;
}

bool Root_window::set_title(const std::string& title) {
    Display* display = static_cast<Display*>(this->display_);
    if (XStoreName(display, DefaultRootWindow(display), title.data()) < 0) {
        return false;
    }
    XFlush(display);
    return true;
}
} // namespace sbar
