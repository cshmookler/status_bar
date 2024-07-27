// Local includes
#include "../build/version.hpp"

namespace sbar {

const char* get_runtime_version() {
    return ::sbar::compiletime_version;
}

} // namespace sbar