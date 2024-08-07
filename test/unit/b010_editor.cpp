#include <gtest/gtest.h>

extern "C" {
#include "b010_editor.h"
#include "stdbool.h"
#include "string.h"
#
}

TEST(b010EditorTests, Strlen) {
    EXPECT_EQ(Strlen(""), 0);
    EXPECT_EQ(Strlen("length"), 6);
}

TEST(b010EditorTests, SubStr) {
    // Test normal substring extraction
    EXPECT_STREQ(SubStr("Hello, World!", 0, 5), "Hello");
    EXPECT_STREQ(SubStr("Hello, World!", 7, 5), "World");

    // Test with start at end of string
    EXPECT_STREQ(SubStr("Hello, World!", 13, 5), "");

    // Test with start beyond end of string
    EXPECT_STREQ(SubStr("Hello, World!", 20, 5), "");

    // Test with count == 0
    EXPECT_STREQ(SubStr("Hello, World!", 0, 0), "");

    // Test with empty string
    EXPECT_STREQ(SubStr("", 0, 5), "");

    // Test with start in middle and count extending past end
    EXPECT_STREQ(SubStr("Hello, World!", 7, 10), "World!");

    EXPECT_STREQ(SubStr("Hello, World!", 0, 1000), "Hello, World!");  // Count larger than string length
}