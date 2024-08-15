#pragma once

/**
 * @file watcher.hpp
 * @author Caden Shmookler (cshmookler@gmail.com)
 * @brief Classes for initializing and creating watches with inotify.
 * @date 2024-08-12
 */

// Standard includes
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <optional>

// External includes
#include <sys/inotify.h>
#include <unistd.h>

// Local includes
#include "utils.hpp"

namespace sbar {

class Inotify;

class Watcher {
    friend Inotify;

    static const int invalid = -1;
    int inotify_ = invalid;
    int watcher_ = invalid;

    [[nodiscard]] bool inotify_good_() const {
        return this->inotify_ >= 0;
    }

    [[nodiscard]] bool watcher_good_() const {
        return this->watcher_ >= 0;
    }

    Watcher(int inotify, const char* path) {
        this->inotify_ = inotify;
        if (! this->inotify_good_()) {
            return;
        }
        if (! std::filesystem::exists(path)) {
            std::ofstream file{ path };
        }
        this->watcher_ =
          inotify_add_watch(this->inotify_, path, IN_CLOSE_WRITE);
        if (! this->watcher_good_()) {
            std::perror("inotify_add_watch");
        }
    }

  public:
    Watcher(const Watcher&) = delete;
    Watcher(Watcher&& watcher) noexcept
    : inotify_(watcher.inotify_), watcher_(watcher.watcher_) {
        watcher.inotify_ = invalid;
        watcher.watcher_ = invalid;
    }
    Watcher& operator=(const Watcher&) = delete;
    Watcher& operator=(Watcher&&) noexcept = default;
    ~Watcher() {
        if (! this->good()) {
            return;
        }
        if (inotify_rm_watch(this->inotify_, watcher_) < 0) {
            std::perror("inotify_rm_watch");
        }
    }

    [[nodiscard]] bool good() const {
        return this->inotify_good_() && this->watcher_good_();
    }

    bool modified() const {
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
};

/**
 * @brief Used to create watches with inotify.
 */
class Inotify {
    friend Public_constructor<Inotify>;

    static std::optional<Public_constructor<Inotify>> global;

    int inotify_ = -1;

    Inotify() {
        this->inotify_ = inotify_init();
        if (! this->good()) {
            std::perror("inotify_init");
            return;
        }
    }

  public:
    static Inotify& get() {
        if (! global.has_value()) {
            global.emplace();
        }
        return global.value();
    }

    Inotify(const Inotify&) = delete;
    Inotify(Inotify&&) noexcept = delete;
    Inotify& operator=(const Inotify&) = delete;
    Inotify& operator=(Inotify&&) noexcept = delete;
    ~Inotify() {
        if (close(this->inotify_) < 0) {
            std::perror("close");
        }
    }

    [[nodiscard]] bool good() const {
        return this->inotify_ >= 0;
    }

    [[nodiscard]] Watcher watch(const char* path) const {
        return Watcher(this->inotify_, path);
    }

    template<typename Rep, typename Period>
    bool has_event(std::chrono::duration<Rep, Period> timeout) {
        auto timeout_us =
          std::chrono::duration_cast<std::chrono::microseconds>(timeout);
        struct timeval timeout_raw {
            0, timeout_us.count()
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
};

std::optional<Public_constructor<Inotify>> Inotify::global;

} // namespace sbar
