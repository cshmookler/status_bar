#pragma once

/*****************************************************************************/
/*  Copyright (c) 2024 Caden Shmookler                                       */
/*                                                                           */
/*  This software is provided 'as-is', without any express or implied        */
/*  warranty. In no event will the authors be held liable for any damages    */
/*  arising from the use of this software.                                   */
/*                                                                           */
/*  Permission is granted to anyone to use this software for any purpose,    */
/*  including commercial applications, and to alter it and redistribute it   */
/*  freely, subject to the following restrictions:                           */
/*                                                                           */
/*  1. The origin of this software must not be misrepresented; you must not  */
/*     claim that you wrote the original software. If you use this software  */
/*     in a product, an acknowledgment in the product documentation would    */
/*     be appreciated but is not required.                                   */
/*  2. Altered source versions must be plainly marked as such, and must not  */
/*     be misrepresented as being the original software.                     */
/*  3. This notice may not be removed or altered from any source             */
/*     distribution.                                                         */
/*****************************************************************************/

/**
 * @file watcher.hpp
 * @author Caden Shmookler (cshmookler@gmail.com)
 * @brief Classes for initializing and creating watches with inotify.
 * @date 2024-08-12
 */

// Standard includes
#include <chrono>
#include <optional>

namespace sbar {

class Inotify;

class Watcher {
    friend Inotify;

    static const int invalid = -1;

    int inotify_ = invalid;
    int watcher_ = invalid;

    [[nodiscard]] inline bool inotify_good_() const {
        return this->inotify_ >= 0;
    }

    [[nodiscard]] inline bool watcher_good_() const {
        return this->watcher_ >= 0;
    }

    Watcher(int inotify, const char* path);

  public:
    Watcher(const Watcher&) = delete;
    Watcher(Watcher&& watcher) noexcept
    : inotify_(watcher.inotify_), watcher_(watcher.watcher_) {
        watcher.inotify_ = invalid;
        watcher.watcher_ = invalid;
    }
    Watcher& operator=(const Watcher&) = delete;
    Watcher& operator=(Watcher&&) noexcept = default;
    ~Watcher();

    /**
     * @brief Returns true if this watcher was successfully initialized and
     * false otherwise.
     */
    [[nodiscard]] inline bool good() const {
        return this->inotify_good_() && this->watcher_good_();
    }

    /**
     * @brief Returns true if the file or directory watched by this instance was
     * modified since the last call to this method.
     */
    bool modified() const;
};

/**
 * struct Public_constructor - Provides a public constructor for a given type.
 *
 * @tparam T - The type to provide a public constructor for.
 */
template<typename T>
struct Public_constructor : public T {
    template<typename... Args>
    Public_constructor(Args&&... args) : T(std::forward<Args>(args)...) {
    }
};

/**
 * @brief Used to create watches with inotify.
 */
class Inotify {
    friend Public_constructor<Inotify>;

    static std::optional<Public_constructor<Inotify>> global;

    int inotify_ = -1;

    [[nodiscard]] bool has_event_(std::chrono::microseconds timeout) const;

    Inotify();

  public:
    /**
     * @brief Returns a reference to the global inotify instance.
     */
    static Inotify& get();

    Inotify(const Inotify&) = delete;
    Inotify(Inotify&&) noexcept = delete;
    Inotify& operator=(const Inotify&) = delete;
    Inotify& operator=(Inotify&&) noexcept = delete;
    ~Inotify();

    /**
     * @brief Returns true if inotify was successfully initialized and false
     * otherwise.
     */
    [[nodiscard]] inline bool good() const {
        return this->inotify_ >= 0;
    }

    /**
     * @brief Creates a new watch for a specific path.
     *
     * @param[in] path - The path to watch.
     * @return the constructed Watch instance.
     */
    [[nodiscard]] inline Watcher watch(const char* path) const {
        return Watcher(this->inotify_, path);
    }

    /**
     * @brief Wait for an event to be received by any watcher until the given
     * timeout elapses.
     *
     * @param[in] timeout - The amount of time (std::chrono::duration) to wait
     * for an event.
     * @return true if an event was received and false otherwise.
     */
    template<typename Rep, typename Period>
    inline bool has_event(std::chrono::duration<Rep, Period> timeout) const {
        return this->has_event_(
          std::chrono::duration_cast<std::chrono::microseconds>(timeout));
    }
};

} // namespace sbar
