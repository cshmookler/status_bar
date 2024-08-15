// Standard includes
#include <string>

// External includes
#include <pwd.h>
#include <unistd.h>

// Local includes
#include "../constants.hpp"

namespace sbar {

std::string get_user() {
    auto uid = geteuid();

    struct passwd* passwd_info = getpwuid(uid);
    if (passwd_info == nullptr) {
        return sbar::error_str;
    }

    return passwd_info->pw_name;
}

} // namespace sbar
