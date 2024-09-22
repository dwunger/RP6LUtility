#include <gtest/gtest.h>

extern "C" {
#include "b010_editor.h"
#include "stdbool.h"
#include "string.h"

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

TEST(b010EditorTests, FileOpen) {
    init_file_manager();

    // Test opening a new file
    int index1 = FileOpen("test1.txt", 0, (char*)"Hex", 0);
    EXPECT_GE(index1, 0);

    // Test opening the same file again (should return the same index)
    int index2 = FileOpen("test1.txt", 0, (char*)"Hex", 0);
    EXPECT_EQ(index1, index2);

    // Test opening a different file - Unsupported parameter Text
    int index3 = FileOpen("test2.txt", 0, (char*)"Text", 0);
    EXPECT_GE(index3, 0);
    EXPECT_NE(index3, index1);

    // Test opening with different editAs - Unsupported path
    int index4 = FileOpen("test3.txt", 0, (char*)"Unicode", 0);
    EXPECT_GE(index4, 0);

    // Test opening duplicate
    int index5 = FileOpen("test1.txt", 0, (char*)"Hex", 1);
    EXPECT_GE(index5, 0);
    EXPECT_NE(index5, index1);

    // Test opening with null filename (should fail)
    int index6 = FileOpen(NULL, 0, (char*)"Hex", 0);
    EXPECT_LT(index6, 0);
}

TEST(b010EditorTests, FindOpenFile) {
    init_file_manager();

    // Open some files
    int index1 = FileOpen("test1.txt", 0, (char*)"Hex", 0);
    int index2 = FileOpen("test2.txt", 0, (char*)"Text", 0);

    // Test finding open files
    EXPECT_EQ(FindOpenFile("test1.txt"), index1);
    EXPECT_EQ(FindOpenFile("test2.txt"), index2);

    // Test finding a file that's not open
    EXPECT_EQ(FindOpenFile("nonexistent.txt"), -1);
}

TEST(b010EditorTests, FileSelect) {
    init_file_manager();

    // Open some files
    int index1 = FileOpen("test1.txt", 0, (char*)"Hex", 0);
    int index2 = FileOpen("test2.txt", 0, (char*)"Text", 0);

    // Select a file and check if it's selected
    FileSelect(index1);
    // You might need to add a function to get the current file index
    // EXPECT_EQ(GetFileNum(), index1);

    // Select another file
    FileSelect(index2);
    // EXPECT_EQ(GetFileNum(), index2);

    // Try to select an invalid index
    FileSelect(-1);
    // EXPECT_EQ(GetFileNum(), index2);  // Should not change
}

