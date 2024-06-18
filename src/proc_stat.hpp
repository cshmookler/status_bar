#pragma once

/**
 * @file proc_stat.hpp
 * @author Caden Shmookler (cshmookler@gmail.com)
 * @brief Structures for collecting information from
 * /proc/stat.
 * @date 2024-06-08
 */

// Standard includes
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

namespace status_bar {

class invalid_proc_stat : public std::runtime_error {
    const char* status_;

  public:
    inline invalid_proc_stat(const char* status, const std::string& error)
    : std::runtime_error(error), status_(status) {
    }

    [[nodiscard]] inline const char* status() const {
        return this->status_;
    }
};

class cpu {
    std::string name_;
    std::vector<size_t> entries_;

    // clang-format off
    static const size_t index_start = __LINE__;
  public:
    enum class index : size_t {
        user_mode,
        low_priority_user_mode,
        system_mode,
        idle,
        io_idle,
        interrupt,
        soft_interrupt,
        stolen,
        guest,
        niced_guest,
    };
    static const size_t index_count = __LINE__ - index_start - 4;
    // clang-format on

    cpu();

    [[nodiscard]] size_t get_total() const;

    [[nodiscard]] size_t get_total(const std::vector<index>& indicies) const;
};

} // namespace status_bar
