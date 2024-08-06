// Standard includes
#include <fstream>

// Local includes
#include "utils.hpp"

namespace sbar {

bool remove_prefix(std::string_view& target, const std::string_view& prefix) {
    if (prefix.size() > target.size()) {
        return false;
    }
    bool found_prefix = target.rfind(prefix, 0) == 0;
    if (found_prefix) {
        target = target.substr(prefix.size());
    }
    return found_prefix;
}

bool remove_postfix(std::string_view& target, const std::string_view& postfix) {
    if (postfix.size() > target.size()) {
        return false;
    }
    size_t postfix_pos = target.size() - postfix.size();
    bool found_postfix = target.find(postfix, postfix_pos) == postfix_pos;
    if (found_postfix) {
        target = target.substr(0, target.size() - postfix.size());
    }
    return found_postfix;
}

std::string get_first_line(const std::filesystem::path& path) {
    std::ifstream file{ path };
    std::string first_line;
    if (! std::getline(file, first_line).good()) {
        return sbar::null_str;
    }
    return first_line;
}

std::string_view split(std::string_view& str, char delimiter) {
    size_t delimiter_index = 0;
    std::string_view return_value;
    while (delimiter_index == 0) {
        delimiter_index = str.find(delimiter);
        if (delimiter_index == std::string::npos) {
            return sbar::null_str;
        }
        return_value = str.substr(0, delimiter_index);
        str = str.substr(delimiter_index + 1, str.size() - delimiter_index + 1);
    }
    return return_value;
}

} // namespace sbar
