// Standard includes
#include <chrono>
#include <csignal>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

// External includes
#include <X11/Xlib.h>
#include <argparse/argparse.hpp>
#include <sys/inotify.h>
#include <type_traits>

// Local includes
#include "constants.hpp"
#include "status.hpp"
#include "version.hpp"

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

class Root_window {
    Display* display_;

  public:
    Root_window() : display_(XOpenDisplay(nullptr)) {
        if (! this->good()) {
            std::cerr << "Error: XOpenDisplay: Failed to open display\n";
        }
    }

    Root_window(const Root_window&) = delete;
    Root_window(Root_window&&) noexcept = delete;
    Root_window& operator=(const Root_window&) = delete;
    Root_window& operator=(Root_window&&) noexcept = delete;

    ~Root_window() {
        if (XCloseDisplay(this->display_) < 0) {
            std::cerr << "Error: XCloseDisplay: Failed to close display\n";
        }
    }

    [[nodiscard]] bool good() const {
        return this->display_ != nullptr;
    }

    bool set_title(const char* title) {
        if (XStoreName(this->display_, DefaultRootWindow(this->display_), title)
          < 0) {
            std::cerr << "Error: XStoreName: Allocation failed\n";
            return false;
        }
        XFlush(this->display_);
        return true;
    }
};

[[nodiscard]] std::string format_status(
  std::unique_ptr<sbar::Cpu_state>& cpu_state_info,
  sbar::Battery_state& battery_state_info,
  sbar::Network_data_stats& network_data_stats,
  const std::string& status) {
    sbar::Optional_system system{};
    sbar::Optional_battery battery{};
    sbar::Optional_backlight backlight{};
    sbar::Optional_network network{};
    sbar::Optional_sound_mixer sound_mixer{};

    std::string formatted_status;

    bool found_escape_sequence = false;

    for (char chr : status) {
        if (! found_escape_sequence) {
            if (chr == '/') {
                found_escape_sequence = true;
            } else {
                formatted_status.push_back(chr);
            }
            continue;
        }

        std::string insert;

        switch (chr) {
            case '/':
                insert = "/";
                break;
            case 't':
                insert = sbar::get_time();
                break;
            case 'u':
                insert = system.call(sbar::get_uptime);
                break;
            case 'd':
                insert = sbar::get_disk_percent();
                break;
            case 's':
                insert = system.call(sbar::get_swap_percent);
                break;
            case 'm':
                insert = system.call(sbar::get_memory_percent);
                break;
            case 'c':
                insert = sbar::get_cpu_percent(cpu_state_info);
                break;
            case 'C':
                insert = sbar::get_cpu_temperature();
                break;
            case '1':
                insert = system.call(sbar::get_one_minute_load_average);
                break;
            case '5':
                insert = system.call(sbar::get_five_minute_load_average);
                break;
            case 'f':
                insert = system.call(sbar::get_fifteen_minute_load_average);
                break;
            case 'b':
                insert = battery.call(sbar::get_battery_status);
                break;
            case 'n':
                insert = battery.call(sbar::get_battery_device);
                break;
            case 'B':
                insert = battery.call(sbar::get_battery_percent);
                break;
            case 'T':
                insert = battery.call(
                  sbar::get_battery_time_remaining, battery_state_info);
                break;
            case 'l':
                insert = backlight.call(sbar::get_backlight_percent);
                break;
            case 'S':
                insert = network.call(sbar::get_network_status);
                break;
            case 'N':
                insert = network.call(sbar::get_network_device);
                break;
            case 'w':
                insert = network.call(sbar::get_network_ssid);
                break;
            case 'W':
                insert =
                  network.call(sbar::get_network_signal_strength_percent);
                break;
            case 'U':
                insert =
                  network.call(sbar::get_network_upload, network_data_stats);
                break;
            case 'D':
                insert =
                  network.call(sbar::get_network_download, network_data_stats);
                break;
            case 'v':
                insert = sound_mixer.call(sbar::get_volume_state);
                break;
            case 'V':
                insert = sound_mixer.call(sbar::get_volume_perc);
                break;
            case 'h':
                insert = sound_mixer.call(sbar::get_capture_state);
                break;
            case 'H':
                insert = sound_mixer.call(sbar::get_capture_perc);
                break;
            case 'e':
                insert = sbar::get_microphone_state();
                break;
            case 'a':
                insert = sbar::get_camera_state();
                break;
            case 'x':
                insert = sbar::get_user();
                break;
            case 'k':
                insert = sbar::get_outdated_kernel_indicator();
                break;
            default:
                break;
        }

        formatted_status.append(insert);

        found_escape_sequence = false;
    }

    return formatted_status;
}

int main(int argc, char** argv) {
    // Setup the argument parser
    argparse::ArgumentParser argparser{ "status_bar",
        sbar::get_runtime_version() };

    argparser.add_argument("-p", "--path")
      .metavar("PATH")
      .nargs(1)
      .help("the path to the notification file\n   ")
      .default_value("/tmp/status_bar");

    argparser.add_argument("-s", "--status")
      .metavar("STATUS")
      .nargs(1)
      .help("custom status with the following interpreted sequences:\n"
            "    //    a literal /\n"
            "    /t    current time\n"
            "    /u    uptime\n"
            "    /d    disk usage\n"
            "    /s    swap usage\n"
            "    /m    memory usage\n"
            "    /c    CPU usage\n"
            "    /C    CPU temperature\n"
            "    /1    1 minute load average\n"
            "    /5    5 minute load average\n"
            "    /f    15 minute load average\n"
            "    /b    battery state\n"
            "    /n    battery device\n"
            "    /B    battery percentage\n"
            "    /T    battery time remaining\n"
            "    /l    backlight percentage\n"
            "    /S    network status\n"
            "    /N    network device\n"
            "    /w    network SSID\n"
            "    /W    network strength percentage\n"
            "    /U    network upload\n"
            "    /D    network download\n"
            "    /v    playback (volume) mute\n"
            "    /V    playback (volume) percentage\n"
            "    /h    capture (mic) mute\n"
            "    /H    capture (mic) percentage\n"
            "    /e    microphone state\n"
            "    /a    camera state\n"
            "    /x    user\n"
            "    /k    outdated kernel indicator\n   ")
      .default_value(" /ac /em | /v /V%v /h /H%c | /S /N /w /W%w | /b /n /B%b "
                     "/T /l%l | /c%c /C°C | /m%m /s%s /d%d | /t | /k /x ");

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

    // Retrieve the status string
    auto status = argparser.get<std::string>("--status");

    // Open the X server display
    Root_window root{};
    if (! root.good()) {
        return 1;
    }

    std::unique_ptr<sbar::Cpu_state> cpu_state_info{};
    sbar::Battery_state battery_state_info{};
    sbar::Network_data_stats network_data_stats{};

    while (! done) {
        std::string formatted_status = format_status(
          cpu_state_info, battery_state_info, network_data_stats, status);

        // Set the status as the title of the root window
        if (! root.set_title(formatted_status.data())) {
            return 1;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Reset the root title
    if (! root.set_title("")) {
        return 1;
    }

    return 0;
}
