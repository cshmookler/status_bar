// Standard includes
#include <chrono>
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

[[nodiscard]] std::string format_status(
  std::unique_ptr<sbar::cpu_state>& cpu_state_info,
  sbar::battery_state& battery_state_info,
  sbar::network_state& network_state_info,
  const std::string& status);

template<typename T, typename F, typename... A>
[[nodiscard]] std::string func_or_error(
  F function, const std::optional<T>& optional, A&... args);

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
            "    %%    a literal %\n"
            "    %t    current time\n"
            "    %u    uptime\n"
            "    %d    disk usage\n"
            "    %s    swap usage\n"
            "    %m    memory usage\n"
            "    %c    CPU usage\n"
            "    %C    CPU temperature\n"
            "    %1    1 minute load average\n"
            "    %5    5 minute load average\n"
            "    %f    15 minute load average\n"
            "    %b    battery state\n"
            "    %n    battery device\n"
            "    %B    battery percentage\n"
            "    %T    battery time remaining\n"
            "    %l    backlight percentage\n"
            "    %S    network status\n"
            "    %N    network device\n"
            "    %w    network SSID\n"
            "    %W    network strength percentage\n"
            "    %U    network upload\n"
            "    %D    network download\n"
            "    %p    bluetooth devices\n"
            "    %v    volume mute\n"
            "    %V    volume percentage\n"
            "    %e    microphone state\n"
            "    %a    camera state\n"
            "    %x    user\n"
            "    %k    outdated kernel indicator\n   ")
      .default_value(
        " %a %e | %l%%l | %v %V%%v | %p | %S %W%%w %w | "
        "%b %n %B%%b %T | %c%%c %C°C | %m%%m %s%%s %d%%d | %t | %k %x ");

    // Parse arguments
    try {
        argparser.parse_args(argc, argv);
    } catch (const std::exception& err) {
        std::cerr << err.what() << "\n\n";
        std::cerr << argparser;
        return 1;
    }

    auto status = argparser.get<std::string>("--status");

    // Open the X server display
    Display* display = XOpenDisplay(nullptr);
    if (display == nullptr) {
        std::cerr << "Error: XOpenDisplay: Failed to open display\n";
        return 1;
    }

    std::unique_ptr<sbar::cpu_state> cpu_state_info{};
    sbar::battery_state battery_state_info{};
    sbar::network_state network_state_info{};

    while (true) {
        std::string formatted_status = format_status(
          cpu_state_info, battery_state_info, network_state_info, status);

        if (XStoreName(
              display, DefaultRootWindow(display), formatted_status.data())
          < 0) {
            std::cerr << "Error: XStoreName: Allocation failed\n";
            return 1;
        }
        XFlush(display);

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Close the X server display
    if (XCloseDisplay(display) < 0) {
        std::cerr << "Error: XCloseDisplay: Failed to close display\n";
        return 1;
    }

    return 0;
}

std::string format_status(std::unique_ptr<sbar::cpu_state>& cpu_state_info,
  sbar::battery_state& battery_state_info,
  sbar::network_state& network_state_info,
  const std::string& status) {
    auto battery = sbar::get_battery();
    auto network = sbar::get_network();

    std::string formatted_status;

    bool found_escape_sequence = false;

    for (char chr : status) {
        if (! found_escape_sequence) {
            if (chr == '%') {
                found_escape_sequence = true;
            } else {
                formatted_status.push_back(chr);
            }
            continue;
        }

        std::string insert;

        switch (chr) {
            case '%':
                insert = "%";
                break;
            case 't':
                insert = sbar::get_time();
                break;
            case 'u':
                insert = sbar::get_uptime();
                break;
            case 'd':
                insert = sbar::get_disk_percent();
                break;
            case 's':
                insert = sbar::get_swap_percent();
                break;
            case 'm':
                insert = sbar::get_memory_percent();
                break;
            case 'c':
                insert = sbar::get_cpu_percent(cpu_state_info);
                break;
            case 'C':
                insert = sbar::get_cpu_temperature();
                break;
            case '1':
                insert = sbar::get_one_minute_load_average();
                break;
            case '5':
                insert = sbar::get_five_minute_load_average();
                break;
            case 'f':
                insert = sbar::get_fifteen_minute_load_average();
                break;
            case 'b':
                insert = func_or_error(sbar::get_battery_status, battery);
                break;
            case 'n':
                insert = func_or_error(sbar::get_battery_device, battery);
                break;
            case 'B':
                insert = func_or_error(sbar::get_battery_percent, battery);
                break;
            case 'T':
                insert = func_or_error(sbar::get_battery_time_remaining,
                  battery,
                  battery_state_info);
                break;
            case 'l':
                insert = sbar::get_backlight_percent();
                break;
            case 'S':
                insert = func_or_error(sbar::get_network_status, network);
                break;
            case 'N':
                insert = func_or_error(sbar::get_network_device, network);
                break;
            case 'w':
                insert = func_or_error(sbar::get_network_ssid, network);
                break;
            case 'W':
                insert = func_or_error(
                  sbar::get_network_signal_strength_percent, network);
                break;
            case 'U':
                insert = func_or_error(
                  sbar::get_network_upload, network, network_state_info);
                break;
            case 'D':
                insert = func_or_error(
                  sbar::get_network_download, network, network_state_info);
                break;
            case 'p':
                insert = sbar::get_bluetooth_devices();
                break;
            case 'v':
                insert = sbar::get_volume_status();
                break;
            case 'V':
                insert = sbar::get_volume_perc();
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

template<typename T, typename F, typename... A>
std::string func_or_error(
  F function, const std::optional<T>& optional, A&... args) {
    static_assert(std::is_invocable_v<F, T, A&...>,
      "The function must be invocable with the given optional value and "
      "arguments");

    if (optional.has_value()) {
        return function(optional.value(), args...);
    }
    return sbar::error_str;
}
