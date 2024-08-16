// Standard includes
#include <cstdio>
#include <filesystem>
#include <fstream>

// External includes
#include <sys/inotify.h>
#include <unistd.h>

// Local includes
#include "../include/watcher.hpp"

namespace sbar {

Watcher::Watcher(int inotify, const char* path) {
    this->inotify_ = inotify;
    if (! this->inotify_good_()) {
        return;
    }
    if (! std::filesystem::exists(path)) {
        std::ofstream file{ path };
    }
    this->watcher_ = inotify_add_watch(this->inotify_, path, IN_CLOSE_WRITE);
    if (! this->watcher_good_()) {
        std::perror("inotify_add_watch");
    }
}

Watcher::~Watcher() {
    if (! this->good()) {
        return;
    }
    if (inotify_rm_watch(this->inotify_, watcher_) < 0) {
        std::perror("inotify_rm_watch");
    }
}

bool Watcher::modified() const {
    if (! this->good()) {
        return false;
    }
    struct inotify_event event {};
    const size_t event_size = sizeof(struct inotify_event);
    size_t bytes_read = read(this->inotify_, &event, event_size);
    if (bytes_read < 0) {
        perror("read");
        return false;
    }
    return bytes_read != 0;
}

bool Inotify::has_event_(std::chrono::microseconds timeout) const {
    if (! this->good()) {
        return false;
    }
    struct timeval timeout_raw {
        0, timeout.count()
    };
    fd_set set;
    FD_ZERO(&set);
    FD_SET(this->inotify_, &set);
    int return_code =
      select(this->inotify_ + 1, &set, nullptr, nullptr, &timeout_raw);
    if (return_code < 0) {
        if (errno != EINTR) {
            perror("select");
        }
        return false;
    }
    if (return_code == 0) {
        // timed out
        return false;
    }
    return FD_ISSET(this->inotify_, &set);
}

Inotify::Inotify() {
    this->inotify_ = inotify_init();
    if (! this->good()) {
        std::perror("inotify_init");
        return;
    }
}

Inotify& Inotify::get() {
    if (! global.has_value()) {
        global.emplace();
    }
    return global.value();
}

Inotify::~Inotify() {
    if (close(this->inotify_) < 0) {
        std::perror("close");
    }
}

std::optional<Public_constructor<Inotify>> Inotify::global;

} // namespace sbar
