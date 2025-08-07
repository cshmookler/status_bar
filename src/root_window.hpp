#pragma once

// Standard includes
#include <string>

// External includes
#include <cpp_result/all.hpp>

namespace sbar {

class root_window_t;

/**
 * @brief Return a new root window object or an error.
 */
[[nodiscard]] res::optional_t<root_window_t> get_root_window();

/**
 * @brief Used for interacting with the root window on the X server.
 *
 * @code{.cpp}
 * auto root_window = get_root_window();
 * if (root_window.failure()) {
 *     std::cerr << root_window.error() << std::endl;
 *     return 1;
 * }
 *
 * root_window->set_title("New title for the root window");
 * @endcode
 */
class root_window_t {
    void* display_; // Xlib Display

    root_window_t(void* display);

    friend res::optional_t<root_window_t> get_root_window();

  public:
    root_window_t(const root_window_t&) = delete;
    root_window_t(root_window_t&&) noexcept;
    root_window_t& operator=(const root_window_t&) = delete;
    root_window_t& operator=(root_window_t&&) noexcept = default;

    ~root_window_t();

    /**
     * @brief Set the title of the root window.
     *
     * @param[in] title - The new title represented as a string.
     * @return a result indicating success or failure.
     */
    res::result_t set_title(const std::string& title);
};

} // namespace sbar
