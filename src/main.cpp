// Standard includes
#include <chrono>
#include <csignal>
#include <cstring>
#include <iostream>
#include <string>

// External includes
#include <argparse/argparse.hpp>
#include <sys/inotify.h>
#include <sys/select.h>
#include <unistd.h>

// Local includes
#include "../include/watcher.hpp"
#include "status.hpp"
#include "root_window.hpp"
#include "version.hpp"

using System_clock = std::chrono::system_clock;
using Time_point = System_clock::time_point;
using Duration = System_clock::duration;
using Milliseconds = std::chrono::milliseconds;
using Seconds = std::chrono::seconds;

bool done = false;

void signal_handler(int signal) {
    switch (signal) {
        case SIGINT:
            /* fallthrough */
        case SIGTERM:
            done = true;
            break;
        case SIGSEGV:
            done = true;
            std::cerr << "Error: Segmentation fault\n";
            break;
        default:
            break;
    }
}

std::optional<sbar::field> get_field(char seq) {
    switch (seq) {
        case 'T': // time
            return sbar::field::time;
        case 'Y': // (uptime)
            return sbar::field::uptime;
        case 'I': // (disk)
            return sbar::field::disk;
        case 'S': // swap
            return sbar::field::swap;
        case 'M': // memory
            return sbar::field::memory;
        case 'C': // cpu
            return sbar::field::cpu;
        case 'P': // (temperature)
            return sbar::field::cpu_temp;
        case '1': // 1st load average
            return sbar::field::load_1;
        case '2': // 2nd load average
            return sbar::field::load_5;
        case '3': // 3rd load average
            return sbar::field::load_15;
        case 'a': // (battery status)
            return sbar::field::battery_status;
        case 'A': // (battery device)
            return sbar::field::battery_device;
        case 'B': // battery
            return sbar::field::battery;
        case 'R': // (battery time) remaining
            return sbar::field::battery_time;
        case 'L': // (back)light
            return sbar::field::backlight;
        case 'e': // (network status)
            return sbar::field::network_status;
        case 'E': // (network device)
            return sbar::field::network_device;
        case 'N': // network
            return sbar::field::network_ssid;
        case 'W': // (wifi) (network strength)
            return sbar::field::network_strength;
        case 'U': // upload
            return sbar::field::network_upload;
        case 'D': // download
            return sbar::field::network_download;
        case 'v': // volume (status)
            return sbar::field::volume_status;
        case 'V': // volume
            return sbar::field::volume;
        case 'h': // (capture status)
            return sbar::field::capture_status;
        case 'H': // (capture)
            return sbar::field::capture;
        case 'm': // microphone (status)
            return sbar::field::microphone;
        case 'c': // camera (status)
            return sbar::field::camera;
        case 'Z': // (user)
            return sbar::field::user;
        case 'k': // kernel (status)
            return sbar::field::kernel_status;
        default:
            return std::nullopt;
    }
}

[[nodiscard]] sbar::Status parse_status(const std::string& status_seq) {
    sbar::Status status;
    status.separators.push_back(""); // At least one separator is required.

    bool found_escape_sequence = false;

    for (char chr : status_seq) {
        if (! found_escape_sequence) {
            if (chr == '/') {
                found_escape_sequence = true;
            } else {
                status.separators.back().push_back(chr);
            }
            continue;
        }
        found_escape_sequence = false;

        std::optional<sbar::field> optional_field = get_field(chr);
        if (! optional_field.has_value()) {
            continue;
        }
        status.active_fields.push_back(optional_field.value());
        status.separators.push_back("");
    }

    return status;
}

int main(int argc, char** argv) {
    // Setup the argument parser
    argparse::ArgumentParser argparser{ "status_bar",
        sbar::get_runtime_version(),
        argparse::default_arguments::all,
        true };

    argparser.add_description("Status bar for dwm (https://dwm.suckless.org). "
                              "Customizable at runtime and updates instantly.");

    argparser.add_argument("-s", "--status")
      .metavar("STATUS")
      .nargs(1)
      .help("custom status with the following interpreted sequences:\n"
            "    //    a literal /\n"
            "    /T    current time\n"
            "    /Y    uptime\n"
            "    /I    disk usage\n"
            "    /S    swap usage\n"
            "    /M    memory usage\n"
            "    /C    CPU usage\n"
            "    /P    CPU temperature\n"
            "    /1    1 minute load average\n"
            "    /2    5 minute load average\n"
            "    /3    15 minute load average\n"
            "    /a    battery state\n"
            "    /A    battery device\n"
            "    /B    battery percentage\n"
            "    /R    battery time remaining\n"
            "    /L    backlight percentage\n"
            "    /e    network status\n"
            "    /E    network device\n"
            "    /N    network SSID\n"
            "    /W    network strength percentage\n"
            "    /U    network upload\n"
            "    /D    network download\n"
            "    /v    playback (volume) mute\n"
            "    /V    playback (volume) percentage\n"
            "    /h    capture (mic) mute\n"
            "    /H    capture (mic) percentage\n"
            "    /m    microphone state\n"
            "    /c    camera state\n"
            "    /Z    user\n"
            "    /k    outdated kernel indicator\n   ")
      .default_value(" /mm | /v /V%v /h /H%c | /e /E /N /W%w | /a /A /B%b "
                     "/R /L%l | /C%c /P°C | /M%m /S%s /I%d | /T | /k /Z ");

    // Parse arguments
    try {
        argparser.parse_args(argc, argv);
    } catch (const std::exception& err) {
        std::cerr << err.what() << "\n\n";
        std::cerr << argparser;
        return 1;
    }

    // Attempt to set signal handlers (ignore them if they fail to be set)
    // https://en.cppreference.com/w/cpp/utility/program/signal
    std::signal(SIGINT, signal_handler);
    std::signal(SIGSEGV, signal_handler);
    std::signal(SIGTERM, signal_handler);

    // Parse the status string
    sbar::Status status = parse_status(argparser.get<std::string>("--status"));

    // Initialize fields
    sbar::Fields fields;

    // Open the X server display
    sbar::Root_window root{};
    if (! root.good()) {
        return 1;
    }

    // Initialize inotify and begin watching the notification file
    sbar::Inotify& inotify = sbar::Inotify::get();
    sbar::Watcher watcher = inotify.watch(sbar::notify_path);

    const Milliseconds inotify_timeout = Milliseconds(10);

    sbar::field fields_to_update = sbar::field_all;
    bool update_now = false;

    Time_point time_at_next_update = System_clock::now();

    while (! done) {
        Time_point now = System_clock::now();
        if (now >= time_at_next_update) {
            time_at_next_update = now + Seconds(1);
            update_now = true;
            fields_to_update = sbar::field_all;
        }

        if (update_now) {
            update_now = false;

            // Format the parsed status
            std::string formatted_status =
              fields.format_status(status, fields_to_update);

            // Set the status as the title of the root window
            if (! root.set_title(formatted_status)) {
                std::cerr << "Failed to set the root window title\n";
                return 1;
            }
        }

        if (! inotify.has_event(inotify_timeout)) {
            continue;
        }
        if (! watcher.modified()) {
            continue;
        }

        std::optional<sbar::field> optional_fields = sbar::get_notification();
        if (! optional_fields.has_value()) {
            std::cerr << "Failed to read the notification file at \""
                      << sbar::notify_path << "\"\n";
            continue;
        }
        update_now = true;
        fields_to_update = optional_fields.value();
    }

    // Clear the root title
    if (! root.set_title("")) {
        std::cerr << "Failed to clear the root window title\n";
        return 1;
    }

    return 0;
}
