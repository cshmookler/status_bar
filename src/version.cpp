// Local includes
#include "../build/version.h"

extern "C" {

const char* sbar_get_runtime_version() {
    return sbar_compiletime_version;
}

} // extern "C"
