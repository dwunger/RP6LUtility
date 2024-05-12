#include <gtest/gtest.h>

extern "C" {
#include "b010_editor.h"
#include "stdbool.h"
}


TEST(b010EditorTests, Strlen) {
    EXPECT_EQ(Strlen(""), 0);
    EXPECT_EQ(Strlen("length"), 6);
}
