// Standard includes
#include <cstring>

// External includes
#include <inotify_ipc/iipc.hpp>

// Local includes
#include "../include/notify.h"
#include "channel.hpp"

extern "C" {

int sbar_notify(sbar_top_field_t fields, char** error) {
    auto channel = iipc::get_channel(sbar::channel);
    if (channel.has_error()) {
        if (error != NULL) {
            *error = strdup(channel.error().string().c_str());
        }
        return 1;
    }

    auto send_result = channel->send(std::to_string(fields));
    if (send_result.failure()) {
        if (error != NULL) {
            *error = strdup(send_result.error().string().c_str());
        }
        return 1;
    }

    return 0;
}

void sbar_error_free(char* error) {
    free(error);
}

} // extern "C"
