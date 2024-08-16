// Standard includes
#include <chrono>
#include <csignal>
#include <cstring>
#include <iostream>
#include <string>

// External includes
#include <X11/Xlib.h>
#include <argparse/argparse.hpp>
#include <sys/inotify.h>
#include <sys/select.h>
#include <type_traits>
#include <unistd.h>

// Local includes
#include "../include/watcher.hpp"
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

template<typename Function, typename... Args>
[[nodiscard]] std::string get_field_or_call(
  std::array<std::string, sbar::field_count>& fields,
  sbar::field fields_to_update,
  sbar::field this_field,
  Function function,
  Args&... args) {
    static_assert(std::is_invocable_v<Function, Args&...>,
      "The function must be invocable with the given arguments");
    size_t field_index = sbar::index(static_cast<size_t>(this_field));
    if ((fields_to_update & this_field) != sbar::field_none) {
        fields.at(field_index) = function(args...);
    }
    return fields.at(field_index);
}

[[nodiscard]] std::string format_status(const std::string& status,
  std::array<std::string, sbar::field_count>& fields,
  sbar::field fields_to_update,
  sbar::System& system,
  sbar::Cpu& cpu,
  sbar::Battery& battery,
  sbar::Backlight& backlight,
  sbar::Network& network,
  sbar::Sound_mixer& sound_mixer) {
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
                insert = get_field_or_call(
                  fields, fields_to_update, sbar::field::time, sbar::get_time);
                break;
            case 'u':
                insert = get_field_or_call(fields,
                  fields_to_update,
                  sbar::field::uptime,
                  sbar::get_uptime,
                  system);
                break;
            case 'd':
                insert = get_field_or_call(fields,
                  fields_to_update,
                  sbar::field::disk,
                  sbar::get_disk_percent);
                break;
            case 's':
                insert = get_field_or_call(fields,
                  fields_to_update,
                  sbar::field::swap,
                  sbar::get_swap_percent,
                  system);
                break;
            case 'm':
                insert = get_field_or_call(fields,
                  fields_to_update,
                  sbar::field::memory,
                  sbar::get_memory_percent,
                  system);
                break;
            case 'c':
                insert = get_field_or_call(fields,
                  fields_to_update,
                  sbar::field::cpu,
                  sbar::get_cpu_percent,
                  cpu);
                break;
            case 'C':
                insert = get_field_or_call(fields,
                  fields_to_update,
                  sbar::field::cpu_temp,
                  sbar::get_cpu_temperature);
                break;
            case '1':
                insert = get_field_or_call(fields,
                  fields_to_update,
                  sbar::field::load_1,
                  sbar::get_one_minute_load_average,
                  system);
                break;
            case '5':
                insert = get_field_or_call(fields,
                  fields_to_update,
                  sbar::field::load_5,
                  sbar::get_five_minute_load_average,
                  system);
                break;
            case 'f':
                insert = get_field_or_call(fields,
                  fields_to_update,
                  sbar::field::load_15,
                  sbar::get_fifteen_minute_load_average,
                  system);
                break;
            case 'b':
                insert = get_field_or_call(fields,
                  fields_to_update,
                  sbar::field::battery_status,
                  sbar::get_battery_status,
                  battery);
                break;
            case 'n':
                insert = get_field_or_call(fields,
                  fields_to_update,
                  sbar::field::battery_device,
                  sbar::get_battery_device,
                  battery);
                break;
            case 'B':
                insert = get_field_or_call(fields,
                  fields_to_update,
                  sbar::field::battery,
                  sbar::get_battery_percent,
                  battery);
                break;
            case 'T':
                insert = get_field_or_call(fields,
                  fields_to_update,
                  sbar::field::battery_time,
                  sbar::get_battery_time_remaining,
                  battery);
                break;
            case 'l':
                insert = get_field_or_call(fields,
                  fields_to_update,
                  sbar::field::backlight,
                  sbar::get_backlight_percent,
                  backlight);
                break;
            case 'S':
                insert = get_field_or_call(fields,
                  fields_to_update,
                  sbar::field::network_status,
                  sbar::get_network_status,
                  network);
                break;
            case 'N':
                insert = get_field_or_call(fields,
                  fields_to_update,
                  sbar::field::network_device,
                  sbar::get_network_device,
                  network);
                break;
            case 'w':
                insert = get_field_or_call(fields,
                  fields_to_update,
                  sbar::field::network_ssid,
                  sbar::get_network_ssid,
                  network);
                break;
            case 'W':
                insert = get_field_or_call(fields,
                  fields_to_update,
                  sbar::field::network_strength,
                  sbar::get_network_signal_strength_percent,
                  network);
                break;
            case 'U':
                insert = get_field_or_call(fields,
                  fields_to_update,
                  sbar::field::network_upload,
                  sbar::get_network_upload,
                  network);
                break;
            case 'D':
                insert = get_field_or_call(fields,
                  fields_to_update,
                  sbar::field::network_download,
                  sbar::get_network_download,
                  network);
                break;
            case 'v':
                insert = get_field_or_call(fields,
                  fields_to_update,
                  sbar::field::volume_status,
                  sbar::get_volume_status,
                  sound_mixer);
                break;
            case 'V':
                insert = get_field_or_call(fields,
                  fields_to_update,
                  sbar::field::volume,
                  sbar::get_volume_perc,
                  sound_mixer);
                break;
            case 'h':
                insert = get_field_or_call(fields,
                  fields_to_update,
                  sbar::field::capture_status,
                  sbar::get_capture_status,
                  sound_mixer);
                break;
            case 'H':
                insert = get_field_or_call(fields,
                  fields_to_update,
                  sbar::field::capture,
                  sbar::get_capture_perc,
                  sound_mixer);
                break;
            case 'e':
                insert = get_field_or_call(fields,
                  fields_to_update,
                  sbar::field::microphone,
                  sbar::get_microphone_status);
                break;
            case 'a':
                insert = get_field_or_call(fields,
                  fields_to_update,
                  sbar::field::camera,
                  sbar::get_camera_status);
                break;
            case 'x':
                insert = get_field_or_call(
                  fields, fields_to_update, sbar::field::user, sbar::get_user);
                break;
            case 'k':
                insert = get_field_or_call(fields,
                  fields_to_update,
                  sbar::field::kernel_status,
                  sbar::get_outdated_kernel_indicator);
                break;
            default:
                break;
        }

        formatted_status.append(insert);

        found_escape_sequence = false;
    }

    system.reset();
    battery.reset();
    backlight.reset();
    network.reset();
    sound_mixer.reset();

    return formatted_status;
}

int main(int argc, char** argv) {
    // Setup the argument parser
    argparse::ArgumentParser argparser{ "status_bar",
        sbar::get_runtime_version(),
        argparse::default_arguments::all,
        true };

    argparser.add_description("Status bar for dwm (https://dwm.suckless.org). "
                              "Customizable at runtime and updates instantly.");

    // argparser.add_argument("-p", "--path")
    //   .metavar("PATH")
    //   .nargs(1)
    //   .help("the path to the notification file\n   ")
    //   .default_value("/tmp/status_bar");

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
      .default_value(" /em | /v /V%v /h /H%c | /S /N /w /W%w | /b /n /B%b "
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

    std::array<std::string, sbar::field_count> fields{};

    sbar::System system;
    sbar::Cpu cpu;
    sbar::Battery battery;
    sbar::Backlight backlight;
    sbar::Network network;
    sbar::Sound_mixer sound_mixer;

    sbar::Inotify& inotify = sbar::Inotify::get();
    sbar::Watcher watcher = inotify.watch(sbar::notify_path);

    while (! done) {
        std::string formatted_status = format_status(status,
          fields,
          sbar::field_all,
          system,
          cpu,
          battery,
          backlight,
          network,
          sound_mixer);

        // Set the status as the title of the root window
        if (! root.set_title(formatted_status.data())) {
            return 1;
        }

        for (size_t i = 0; i < 20; i++) {
            if (! inotify.has_event(std::chrono::milliseconds(50))) {
                continue;
            }
            if (! watcher.modified()) {
                continue;
            }

            std::optional<sbar::field> optional_fields =
              sbar::get_notification();
            if (! optional_fields.has_value()) {
                std::cerr << "Failed to read the notification file: \""
                          << sbar::notify_path << "\"\n";
                continue;
            }

            std::string new_formatted_status = format_status(status,
              fields,
              optional_fields.value(),
              system,
              cpu,
              battery,
              backlight,
              network,
              sound_mixer);

            // Set the status as the title of the root window
            if (! root.set_title(new_formatted_status.data())) {
                return 1;
            }
        }
    }

    // Reset the root title
    if (! root.set_title("")) {
        return 1;
    }

    return 0;
}
