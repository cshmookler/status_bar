// Local includes
#include "../src/version.hpp"

namespace status_bar {

const char* get_runtime_version() {
    return ::status_bar::compiletime_version;
}

} // namespace status_bar