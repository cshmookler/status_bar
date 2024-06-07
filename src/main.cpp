// Standard includes
#include <chrono>
#include <iostream>
#include <string>
#include <thread>

// External includes
#include <X11/Xlib.h>
#include <argparse/argparse.hpp>
#include <sys/inotify.h>

// Local includes
#include "status.hpp"
#include "version.hpp"

int loop(Display* display, const std::string& status);

[[nodiscard]] std::string format_status(const std::string& status);

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
                  "    %D    disk usage\n"
                  "    %S    swap usage\n"
                  "    %M    memory usage\n"
                  "    %C    CPU usage\n"
                  "    %b    battery state\n"
                  "    %B    battery percentage\n"
                  "    %L    backlight percentage\n"
                  "    %w    network SSID\n"
                  "    %W    WIFI percentage\n"
                  "    %P    bluetooth devices\n"
                  "    %v    volume mute\n"
                  "    %V    volume percentage\n"
                  "    %E    microphone state\n"
                  "    %A    camera state\n   ")
            .default_value(" %V%%v | %v%%m | %P | %W%%w | %w | %L%%l | %B%%b | "
                           "%b | %C%%c | %M%%m | %S%%s | %D%%d | %T");

    // Parse arguments
    try {
        argparser.parse_args(argc, argv);
    }
    catch (const std::exception& err) {
        std::cerr << err.what() << "\n\n";
        std::cerr << argparser;
        std::exit(1);
    }

    // Open the X server display
    Display* display = XOpenDisplay(nullptr);
    if (display == nullptr) {
        std::cout << "Error: XOpenDisplay: Failed to open display\n";
        return 1;
    }

    int return_val = loop(display, argparser.get<std::string>("--status"));

    // Close the X server display
    if (XCloseDisplay(display) < 0) {
        std::cout << "Error: XCloseDisplay: Failed to close display\n";
        return 1;
    }

    return return_val;
}

int loop(Display* display, const std::string& status) {
    while (true) {
        std::string formatted_status = format_status(status);

        std::cout << formatted_status << '\n';

        if (XStoreName(
                    display,
                    DefaultRootWindow(display),
                    formatted_status.data())
            < 0) {
            std::cout << "Error: XStoreName: Allocation failed\n";
            return 1;
        }
        XFlush(display);

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

std::string format_status(const std::string& status) {
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
            case '%': insert = "%"; break;
            case 't': insert = status_bar::time(); break;
            case 'u': insert = status_bar::uptime(); break;
            case 'D': insert = status_bar::disk_percent(); break;
            case 'S': insert = status_bar::swap_percent(); break;
            case 'M': insert = status_bar::memory_percent(); break;
            case 'C': insert = status_bar::cpu_percent(); break;
            case 'b': insert = status_bar::battery_state(); break;
            case 'B': insert = status_bar::battery_perc(); break;
            case 'L': insert = status_bar::backlight_perc(); break;
            case 'w': insert = status_bar::network_ssid(); break;
            case 'W': insert = status_bar::wifi_perc(); break;
            case 'P': insert = status_bar::bluetooth_devices(); break;
            case 'v': insert = status_bar::volume_status(); break;
            case 'V': insert = status_bar::volume_perc(); break;
            case 'E': insert = status_bar::microphone_state(); break;
            case 'A': insert = status_bar::camera_state(); break;
            default: break;
        }

        formatted_status.append(insert);

        found_escape_sequence = false;
    }

    return formatted_status;
}
