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

// Local includes
#include "proc_stat.hpp"
#include "status.hpp"
#include "version.hpp"

int loop(Display* display,
  std::unique_ptr<status_bar::cpu>& cpu_stat,
  const std::string& status);

[[nodiscard]] std::string format_status(
  std::unique_ptr<status_bar::cpu>& cpu_stat, const std::string& status);

int main(int argc, char** argv) {
    // Setup the argument parser
    argparse::ArgumentParser argparser{ "status_bar",
        status_bar::get_runtime_version() };

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
            "    %B    battery percentage\n"
            "    %l    backlight percentage\n"
            "    %w    network SSID\n"
            "    %W    WIFI percentage\n"
            "    %p    bluetooth devices\n"
            "    %v    volume mute\n"
            "    %V    volume percentage\n"
            "    %e    microphone state\n"
            "    %a    camera state\n   ")
      .default_value(" %V%%v | %v%%m | %p | %W%%w | %w | %l%%l | %B%%b | "
                     "%b | %1%%i %5%%v %f%%xv | %C | %c%%c | %m%%m | %s%%s "
                     "| %d%%d | %t ");

    // Parse arguments
    try {
        argparser.parse_args(argc, argv);
    } catch (const std::exception& err) {
        std::cerr << err.what() << "\n\n";
        std::cerr << argparser;
        std::exit(1);
    }

    // Open the X server display
    Display* display = XOpenDisplay(nullptr);
    if (display == nullptr) {
        std::cerr << "Error: XOpenDisplay: Failed to open display\n";
        return 1;
    }

    std::unique_ptr<status_bar::cpu> cpu_stat;

    int return_val =
      loop(display, cpu_stat, argparser.get<std::string>("--status"));

    // Close the X server display
    if (XCloseDisplay(display) < 0) {
        std::cerr << "Error: XCloseDisplay: Failed to close display\n";
        return 1;
    }

    return return_val;
}

int loop(Display* display,
  std::unique_ptr<status_bar::cpu>& cpu_stat,
  const std::string& status) {
    while (true) {
        std::string formatted_status = format_status(cpu_stat, status);

        if (XStoreName(
              display, DefaultRootWindow(display), formatted_status.data())
          < 0) {
            std::cerr << "Error: XStoreName: Allocation failed\n";
            return 1;
        }
        XFlush(display);

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

std::string format_status(
  std::unique_ptr<status_bar::cpu>& cpu_stat, const std::string& status) {
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
                insert = status_bar::time();
                break;
            case 'u':
                insert = status_bar::uptime();
                break;
            case 'd':
                insert = status_bar::disk_percent();
                break;
            case 's':
                insert = status_bar::swap_percent();
                break;
            case 'm':
                insert = status_bar::memory_percent();
                break;
            case 'c':
                insert = status_bar::cpu_percent(cpu_stat);
                break;
            case 'C':
                insert = status_bar::cpu_temperature();
                break;
            case '1':
                insert = status_bar::one_minute_load_average();
                break;
            case '5':
                insert = status_bar::five_minute_load_average();
                break;
            case 'f':
                insert = status_bar::fifteen_minute_load_average();
                break;
            case 'b':
                insert = status_bar::battery_state();
                break;
            case 'B':
                insert = status_bar::battery_percent();
                break;
            case 'l':
                insert = status_bar::backlight_percent();
                break;
            case 'w':
                insert = status_bar::network_ssid();
                break;
            case 'W':
                insert = status_bar::wifi_percent();
                break;
            case 'p':
                insert = status_bar::bluetooth_devices();
                break;
            case 'v':
                insert = status_bar::volume_status();
                break;
            case 'V':
                insert = status_bar::volume_perc();
                break;
            case 'e':
                insert = status_bar::microphone_state();
                break;
            case 'a':
                insert = status_bar::camera_state();
                break;
            default:
                break;
        }

        formatted_status.append(insert);

        found_escape_sequence = false;
    }

    return formatted_status;
}
