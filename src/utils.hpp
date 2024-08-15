#pragma once

/**
 * @file utils.hpp
 * @author Caden Shmookler (cshmookler@gmail.com)
 * @brief A collection of helper classes and functions.
 * @date 2024-07-30
 */

// Standard includes
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>

// Local includes
#include "constants.hpp"

namespace sbar {

/**
 * @brief Attempts to format a given string using std::sprintf.
 * Returns an error string if the formatting fails.
 *
 * @tparam Args - The types of the arguments for formatting method.
 * @param[in] format - The string to be formatted.
 * @param[in] args - The arguments for the formatting method.
 */
template<typename... Args>
[[nodiscard]] std::string sprintf(const char* format, Args... args) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, hicpp-vararg)
    int size = std::snprintf(nullptr, 0, format, args...);
    std::string buffer(size, '\0');
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, hicpp-vararg)
    if (std::sprintf(buffer.data(), format, args...) < 0) {
        return error_str;
    }
    return buffer;
}

/**
 * @brief Removes a given prefix from a given string.
 *
 * @param[in, out] target - The string to remove the prefix from.
 * @param[in] prefix - The prefix to remove from the given string.
 * @return true if the prefix was successfully removed and false if the prefix
 * was not found.
 */
[[nodiscard]] bool remove_prefix(
  std::string_view& target, const std::string_view& prefix);

/**
 * @brief Removes a given postfix from a given string.
 *
 * @param[in, out] target - The string to remove the postfix from.
 * @param[in] postfix - The postfix to remove from the given string.
 * @return true if the postfix was successfully removed and false if the postfix
 * was not found.
 */
[[nodiscard]] bool remove_postfix(
  std::string_view& target, const std::string_view& postfix);

/**
 * @brief Returns the first line in a given file.
 *
 * @param[in] path - The path of the file to read the first line from.
 */
[[nodiscard]] std::string get_first_line(const std::filesystem::path& path);

/**
 * @brief Splits a given string in half by the first instance of a given
 * delimiter. The left half is returned and the right half replaces the given
 * string.
 *
 * @param[in, out] str - The string to split in half.
 * @param[in] delimiter - The delimiter to search for (from left to right).
 */
[[nodiscard]] std::string_view split(std::string_view& str, char delimiter);

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
 * @brief Used for timing sections of code.
 *
 * @code{.cpp}
 * Stopwatch stopwatch{};
 * // some code...
 * stopwatch.reset();
 * // more code...
 * stopwatch.reset();
 * @endcode
 */
class Stopwatch {
    using System_clock = std::chrono::system_clock;
    using Time_point = System_clock::time_point;
    using Duration = System_clock::duration;

    std::string_view name_;
    Time_point start_;

  public:
    /**
     * @brief Starts a new stopwatch and writes its name to stdout.
     *
     * @param[in] name - The name of this stopwatch.
     */
    explicit inline Stopwatch(const std::string_view& name = "stopwatch")
    : name_(name) {
        std::cout << this->name_ << " start\n";
        this->start_ = System_clock::now();
    }

    /**
     * @brief Resets the stopwatch and writes the time elapsed since the last
     * reset to stdout.
     *
     * @tparam Unit - The unit to use when printing the elapsed time.
     * @param[in] unit_label - The label of the unit (used when printing).
     */
    template<typename Unit = std::chrono::microseconds>
    void reset(const std::string_view& unit_label = "us") {
        Duration elapsed = System_clock::now() - this->start_;
        std::cout << this->name_ << " reset: " << std::setw(5)
                  << std::chrono::duration_cast<Unit>(elapsed).count() << ' '
                  << unit_label << '\n';
        this->start_ = System_clock::now();
    }
};

} // namespace sbar
