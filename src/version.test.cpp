// External includes
#include <gtest/gtest.h>

// Local includes
#include "../src/version.hpp"

TEST(version_test, runtime_version_matches_compiletime_version) {
    ASSERT_STREQ(
      status_bar::get_runtime_version(), status_bar::compiletime_version);
}
