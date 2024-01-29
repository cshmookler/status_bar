// Standard includes
#include <array>
#include <chrono>
#include <ctime>
#include <iostream>
#include <thread>

// External includes
#include <X11/Xlib.h>

// Local includes
#include "version.hpp"

constexpr std::string_view proper_usage = {
    "\n"
    "Options:\n"
    "    -h, --help         # List commands and options\n"
    "    -v, --version      # Show version\n"
    "\n"
};

enum class opt_id {
    none,
    help,
    version,
};

struct opt_t {
    std::array<std::string_view, 2> patterns;
    opt_id id = opt_id::none;
    bool value = false;
};

constexpr std::array<opt_t, 2> options = {
    {
     { { "-h", "--help" }, opt_id::help },
     { { "-v", "--version" }, opt_id::version },
     }
};

int failure(const std::string_view& msg) {
    std::cout << "\n" << msg << "\n" << proper_usage;
    return 1;
}

int success() {
    return 0;
}

int success(const std::string_view& msg) {
    std::cout << msg;
    return success();
}

int main(int argc, char** argv) {
    // Parse arguments
    for (int arg_i = 1; arg_i < argc; arg_i++) {
        bool match_found = false;
        for (const opt_t& opt : options) {
            for (const std::string_view& pattern : opt.patterns) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                if (pattern.compare(argv[arg_i]) != 0) {
                    continue;
                }
                switch (opt.id) {
                    case opt_id::none: break;
                    case opt_id::help: return success(proper_usage);
                    case opt_id::version:
                        return success(status_bar::compiletime_version);
                }
                match_found = true;
                break;
            }
            if (match_found) {
                break;
            }
        }

        if (! match_found) {
            // positional argument
            return failure("Error: Unknown argument");
        }
    }

    // Start the X server
    Display* display = XOpenDisplay(nullptr);
    if (display == nullptr) {
        return failure("Error: XOpenDisplay: Failed to open display");
    }

    while (true) {
        std::string status = " ";

        std::string time_string(100, '\0');
        std::time_t now = std::time(nullptr);
        time_string.resize(std::strftime(
                time_string.data(),
                time_string.size(),
                "%Z %F %a %T",
                std::localtime(&now)));
        status += time_string + " ";

        if (XStoreName(display, DefaultRootWindow(display), status.data())
            < 0) {
            return failure("Error: XStoreName: Allocation failed");
        }
        XFlush(display);

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    if (XCloseDisplay(display) < 0) {
        return failure("Error: XCloseDisplay: Failed to close display");
    }

    return 0;
}
