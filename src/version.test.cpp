// External includes
#include <gtest/gtest.h>

// Local includes
#include "../build/version.hpp"

TEST(versionTest, runtimeVersionMatchesCompilationVersion) {
    ASSERT_STREQ(sbar::get_runtime_version(), sbar::compiletime_version);
}
