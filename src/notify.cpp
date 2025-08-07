// External includes
#include <inotify_ipc/iipc.hpp>

// Local includes
#include "../include/notify.h"
#include "channel.hpp"

extern "C" {

int sbar_notify(sbar_field_t fields) {
    auto channel = iipc::get_channel(sbar::channel);
    if (channel.has_error()) {
        return 1;
    }

    auto send_result = channel->send(std::to_string(fields));
    if (send_result.failure()) {
        return 1;
    }

    return 0;
}

} // extern "C"
