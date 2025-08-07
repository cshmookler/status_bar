// External includes
#include <X11/Xlib.h>

// Local includes
#include "root_window.hpp"

namespace sbar {

res::optional_t<root_window_t> get_root_window() {
    void* display = XOpenDisplay(nullptr);
    if (display == nullptr) {
        return RES_NEW_ERROR(
          "Failed to get a handle to the root window running on the X server.");
    }

    return root_window_t{ display };
}

root_window_t::root_window_t(void* display) : display_(display) {
}

root_window_t::root_window_t(root_window_t&& root_window) noexcept
: display_(root_window.display_) {
    root_window.display_ = nullptr;
}

root_window_t::~root_window_t() {
    if (this->display_ != nullptr) {
        XCloseDisplay(static_cast<Display*>(this->display_));
    }
}

res::result_t root_window_t::set_title(const std::string& title) {
    Display* display = static_cast<Display*>(this->display_);

    if (XStoreName(display, DefaultRootWindow(display), title.data()) < 0) {
        return RES_NEW_ERROR("Failed to set the title of the root window.");
    }
    XFlush(display); // XFlush does not have a documented return value.

    return res::success;
}

} // namespace sbar
