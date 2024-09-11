#pragma once

/**
 * @file root_window.hpp
 * @author Caden Shmookler (cshmookler@gmail.com)
 * @brief Utilities for interacting with the root window under X.
 * @date 2024-09-10
 */

// Standard includes
#include <string>

namespace sbar {

/**
 * @brief Used for interacting with the root window under X.
 *
 * @code{.cpp}
 * Root_window root;
 * if (root.good()) {
 *      root.set_title("New title for the root window");
 * }
 * @endcode
 */
class Root_window {
    void* display_; // Xlib Display

  public:
    Root_window();

    Root_window(const Root_window&) = delete;
    Root_window(Root_window&&) noexcept = delete;
    Root_window& operator=(const Root_window&) = delete;
    Root_window& operator=(Root_window&&) noexcept = default;

    ~Root_window();

    [[nodiscard]] bool good() const;
    bool set_title(const std::string& title);
};

} // namespace sbar
