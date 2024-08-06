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
#include <functional>
#include <iostream>
#include <optional>
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
 * @brief Split a given string into a predefined number of segments separated by
 * a given delimiter.
 *
 * @tparam count - The number of segments to split the given string into.
 * @param[in] str - The string to split into segments.
 * @param[in] delimiter - The delimiter to search for (from left to right)
 */
template<size_t count>
[[nodiscard]] std::array<std::string, count> split(
  std::string_view str, char delimiter) {
    std::array<std::string, count> fields{};
    for (std::string& field : fields) {
        field = split(str, delimiter);
    }
    return fields;
}

/**
 * @brief Converts each element in the given array to an unsigned long long.
 * If an element cannot be converted, then zero (0) is recorded instead.
 *
 * @tparam count - The length of the given array.
 * @param[in] string_fields - An array of strings to convert to integers
 * (unsigned long long).
 */
template<size_t count>
[[nodiscard]] std::array<size_t, count> to_integers(
  const std::array<std::string, count>& string_fields) {
    std::array<size_t, count> integer_fields{};
    for (size_t i = 0; i < count; i++) {
        const std::string& string_field = string_fields[i];
        try {
            integer_fields[i] = std::stoull(string_field);
        } catch (...) {
            integer_fields[i] = 0;
        }
    }
    return integer_fields;
}

/**
 * @brief Attempts to construct an instance of a given type with a custom
 * constructor when the construct() method is called.
 *
 * @tparam T - The type to construct an instance of.
 * @tparam constructor - The custom constructor for the given type.
 */
template<typename T>
class Optional_construction {
    std::optional<T> optional_value_ = std::nullopt;
    bool construction_attempted_ = false;

  protected:
    [[nodiscard]] virtual std::optional<T> constructor_() = 0;

  public:
    Optional_construction() = default;
    Optional_construction(const Optional_construction&) = delete;
    Optional_construction(Optional_construction&&) = default;
    Optional_construction& operator=(const Optional_construction&) = delete;
    Optional_construction& operator=(Optional_construction&&) = default;
    virtual ~Optional_construction() = default;

    bool construct() {
        if (this->optional_value_.has_value()) {
            return true;
        }
        if (construction_attempted_) {
            return false;
        }

        this->construction_attempted_ = true;
        // std::cout << "hgot here 2" << std::endl;
        std::optional<T> new_optional_value = this->constructor_();
        this->optional_value_ = std::move(new_optional_value);
        // std::cout << "hgot here 7" << std::endl;
        return this->optional_value_.has_value();
    }

    T& value() {
        return this->optional_value_.value();
    }

    template<typename Function, typename... Args>
    std::string call(Function function, Args&... args) {
        static_assert(std::is_invocable_v<Function, T, Args&...>,
          "The function must be invocable with the given arguments");
        // std::cout << "hgot here 1" << std::endl;
        if (! this->construct()) {
            // std::cout << "hgot here 8" << std::endl;
            return sbar::error_str;
        }
        // std::cout << "hgot here 9" << std::endl;
        return function(this->optional_value_.value(), args...);
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
