// Standard includes
#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>

// External includes
#include <sys/utsname.h>

// Local includes
#include "../constants.hpp"

namespace sbar {

std::string get_outdated_kernel_indicator() {
    const char* const modules_path = "/usr/lib/modules/";

    utsname utsname_info{};
    if (uname(&utsname_info) != 0) {
        int err = errno;
        std::cerr << "uname(): " << std::strerror(err) << '\n';
        return sbar::error_str;
    }

    std::string running_release{ static_cast<const char*>(
      utsname_info.release) };

    std::string latest_installed_release{};

    for (const std::filesystem::directory_entry& release :
      std::filesystem::directory_iterator(modules_path)) {
        if (! release.is_directory()) {
            continue;
        }

        std::string release_name = release.path().filename();

        if (latest_installed_release.empty()
          || latest_installed_release < release_name) {
            latest_installed_release = release_name;
        }
    }

    if (latest_installed_release.empty()) {
        std::cerr << "No installed kernels found in " << modules_path << '\n';
        return sbar::error_str;
    }

    if (running_release != latest_installed_release) {
        return "🔴";
    }

    return "🟢";
}

} // namespace sbar
