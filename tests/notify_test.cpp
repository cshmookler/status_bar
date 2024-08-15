// Standard includes
#include <cmath>

// External includes
#include <gtest/gtest.h>

// Local includes
#include "../include/notify.hpp"

constexpr size_t bits_per_byte = 8;
constexpr size_t size_bytes = sizeof(size_t);
constexpr size_t size_bits = size_bytes * bits_per_byte;

TEST(notify, bit) {
    for (size_t i = 0; i < size_bits; i++) {
        EXPECT_EQ(sbar::bit(i), std::pow(2, i));
    }
}

TEST(notify, index) {
    for (size_t i = 0; i < size_bits; i++) {
        EXPECT_EQ(i, sbar::index(std::pow(2, i)));
    }
}
