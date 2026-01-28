#include <gtest/gtest.h>
#include <cstdio>
#include <direct.h>

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
    EXPECT_EQ(GetFileNum(), index1);

    // Select another file
    FileSelect(index2);
    EXPECT_EQ(GetFileNum(), index2);

    // Try to select an invalid index
    FileSelect(-1);
    EXPECT_EQ(GetFileNum(), index2);  // Should not change
}

TEST(b010EditorTests, GetFileNum) {
    init_file_manager();

    // No file open initially
    EXPECT_EQ(GetFileNum(), -1);

    // Open a file and verify index
    int index1 = FileOpen("getfilenum_test1.txt", 0, (char*)"Hex", 0);
    EXPECT_EQ(GetFileNum(), index1);

    // Open another file, current should change
    int index2 = FileOpen("getfilenum_test2.txt", 0, (char*)"Hex", 0);
    EXPECT_EQ(GetFileNum(), index2);

    // Select first file back
    FileSelect(index1);
    EXPECT_EQ(GetFileNum(), index1);
}

TEST(b010EditorTests, Strcmp) {
    // Equal strings
    EXPECT_EQ(Strcmp("hello", "hello"), 0);
    EXPECT_EQ(Strcmp("", ""), 0);

    // First string less than second
    EXPECT_LT(Strcmp("abc", "abd"), 0);
    EXPECT_LT(Strcmp("abc", "abcd"), 0);
    EXPECT_LT(Strcmp("", "a"), 0);

    // First string greater than second
    EXPECT_GT(Strcmp("abd", "abc"), 0);
    EXPECT_GT(Strcmp("abcd", "abc"), 0);
    EXPECT_GT(Strcmp("a", ""), 0);
}

TEST(b010EditorTests, Strstr) {
    // Substring found
    EXPECT_STREQ(Strstr("hello world", "world"), "world");
    EXPECT_STREQ(Strstr("hello world", "hello"), "hello world");
    EXPECT_STREQ(Strstr("hello world", "o w"), "o world");

    // Substring not found
    EXPECT_EQ(Strstr("hello world", "xyz"), nullptr);
    EXPECT_EQ(Strstr("hello", "hello world"), nullptr);

    // Empty needle (always matches at start)
    EXPECT_STREQ(Strstr("hello", ""), "hello");

    // Empty haystack
    EXPECT_EQ(Strstr("", "hello"), nullptr);
    EXPECT_STREQ(Strstr("", ""), "");
}

TEST(b010EditorTests, SPrintf) {
    // Format integer
    const char* result1 = SPrintf("%d", 42);
    EXPECT_STREQ(result1, "42");
    free((void*)result1);

    // Format string
    const char* result2 = SPrintf("%s", "hello");
    EXPECT_STREQ(result2, "hello");
    free((void*)result2);

    // Multiple arguments
    const char* result3 = SPrintf("%s %d %s", "test", 123, "end");
    EXPECT_STREQ(result3, "test 123 end");
    free((void*)result3);

    // Hex format
    const char* result4 = SPrintf("0x%08X", 255);
    EXPECT_STREQ(result4, "0x000000FF");
    free((void*)result4);
}

TEST(b010EditorTests, ReadWriteUInt) {
    init_file_manager();

    // Create a test file with some content
    int index = FileOpen("readwrite_uint_test.bin", 0, (char*)"Hex", 0);
    EXPECT_GE(index, 0);

    // Write some uint values
    WriteUInt(0, 0x12345678);
    WriteUInt(4, 0xDEADBEEF);
    WriteUInt(8, 0);
    WriteUInt(12, 0xFFFFFFFF);

    // Read back and verify
    EXPECT_EQ(ReadUInt(0), 0x12345678u);
    EXPECT_EQ(ReadUInt(4), 0xDEADBEEFu);
    EXPECT_EQ(ReadUInt(8), 0u);
    EXPECT_EQ(ReadUInt(12), 0xFFFFFFFFu);

    FileClose();
}

TEST(b010EditorTests, ReadUShort) {
    init_file_manager();

    int index = FileOpen("read_ushort_test.bin", 0, (char*)"Hex", 0);
    EXPECT_GE(index, 0);

    // Write uint that contains two ushorts
    WriteUInt(0, 0x5678ABCD);  // Little endian: CD AB 78 56

    // Read ushorts
    EXPECT_EQ(ReadUShort(0), 0xABCDu);
    EXPECT_EQ(ReadUShort(2), 0x5678u);

    FileClose();
}

TEST(b010EditorTests, ReadUByte) {
    init_file_manager();

    int index = FileOpen("read_ubyte_test.bin", 0, (char*)"Hex", 0);
    EXPECT_GE(index, 0);

    // Write uint that contains four bytes
    WriteUInt(0, 0x12345678);  // Little endian: 78 56 34 12

    // Read bytes
    EXPECT_EQ(ReadUByte(0), 0x78u);
    EXPECT_EQ(ReadUByte(1), 0x56u);
    EXPECT_EQ(ReadUByte(2), 0x34u);
    EXPECT_EQ(ReadUByte(3), 0x12u);

    FileClose();
}

TEST(b010EditorTests, WriteBytes) {
    init_file_manager();

    int index = FileOpen("write_bytes_test.bin", 0, (char*)"Hex", 0);
    EXPECT_GE(index, 0);

    unsigned char data[] = {0x11, 0x22, 0x33, 0x44, 0x55};
    WriteBytes(data, 0, 5);

    // Verify by reading back
    EXPECT_EQ(ReadUByte(0), 0x11u);
    EXPECT_EQ(ReadUByte(1), 0x22u);
    EXPECT_EQ(ReadUByte(2), 0x33u);
    EXPECT_EQ(ReadUByte(3), 0x44u);
    EXPECT_EQ(ReadUByte(4), 0x55u);

    // Write at an offset
    unsigned char data2[] = {0xAA, 0xBB};
    WriteBytes(data2, 10, 2);
    EXPECT_EQ(ReadUByte(10), 0xAAu);
    EXPECT_EQ(ReadUByte(11), 0xBBu);

    FileClose();
}

TEST(b010EditorTests, InsertBytes) {
    init_file_manager();

    int index = FileOpen("insert_bytes_test.bin", 0, (char*)"Hex", 0);
    EXPECT_GE(index, 0);

    // Write initial data
    unsigned char data[] = {0x11, 0x22, 0x33, 0x44};
    WriteBytes(data, 0, 4);

    // Insert 2 bytes at offset 2 with value 0xFF
    InsertBytes(2, 2, 0xFF);

    // Verify: should be 0x11, 0x22, 0xFF, 0xFF, 0x33, 0x44
    EXPECT_EQ(ReadUByte(0), 0x11u);
    EXPECT_EQ(ReadUByte(1), 0x22u);
    EXPECT_EQ(ReadUByte(2), 0xFFu);
    EXPECT_EQ(ReadUByte(3), 0xFFu);
    EXPECT_EQ(ReadUByte(4), 0x33u);
    EXPECT_EQ(ReadUByte(5), 0x44u);

    FileClose();
}

TEST(b010EditorTests, FileNameGetBase) {
    // Windows-style paths
    EXPECT_STREQ(FileNameGetBase("C:\\Users\\test\\file.txt"), "file.txt");
    EXPECT_STREQ(FileNameGetBase("C:\\file.txt"), "file.txt");

    // Unix-style paths
    EXPECT_STREQ(FileNameGetBase("/home/user/file.txt"), "file.txt");

    // Mixed path separators
    EXPECT_STREQ(FileNameGetBase("C:/Users/test\\file.txt"), "file.txt");

    // No path separator
    EXPECT_STREQ(FileNameGetBase("file.txt"), "file.txt");

    // Empty string
    EXPECT_STREQ(FileNameGetBase(""), "");
}

TEST(b010EditorTests, FileNameGetPath) {
    // Windows-style paths
    EXPECT_STREQ(FileNameGetPath("C:\\Users\\test\\file.txt"), "C:\\Users\\test");

    // Unix-style paths
    EXPECT_STREQ(FileNameGetPath("/home/user/file.txt"), "/home/user");

    // No path separator (returns ".")
    EXPECT_STREQ(FileNameGetPath("file.txt"), ".");
}

TEST(b010EditorTests, ReadString) {
    init_file_manager();

    int index = FileOpen("read_string_test.bin", 0, (char*)"Hex", 0);
    EXPECT_GE(index, 0);

    // Write a null-terminated string
    unsigned char data[] = {'H', 'e', 'l', 'l', 'o', '\0', 'W', 'o', 'r', 'l', 'd', '\0'};
    WriteBytes(data, 0, 12);

    // Read first string
    const char* str1 = ReadString(0);
    EXPECT_STREQ(str1, "Hello");
    free((void*)str1);

    // Read second string
    const char* str2 = ReadString(6);
    EXPECT_STREQ(str2, "World");
    free((void*)str2);

    // Read from middle of string
    const char* str3 = ReadString(2);
    EXPECT_STREQ(str3, "llo");
    free((void*)str3);

    FileClose();
}

TEST(b010EditorTests, FileSaveAndClose) {
    init_file_manager();

    // Create and write to a file
    int index = FileOpen("save_close_test.bin", 0, (char*)"Hex", 0);
    EXPECT_GE(index, 0);

    WriteUInt(0, 0xCAFEBABE);
    FileSave();
    FileClose();

    // Verify no file is selected after close
    EXPECT_EQ(GetFileNum(), -1);

    // Reopen and verify content persisted
    int index2 = FileOpen("save_close_test.bin", 0, (char*)"Hex", 0);
    EXPECT_GE(index2, 0);
    EXPECT_EQ(ReadUInt(0), 0xCAFEBABEu);

    FileClose();

    // Clean up test file
    remove("save_close_test.bin");
}

TEST(b010EditorTests, FileSaveRange) {
    init_file_manager();

    // Create source file with data
    int index = FileOpen("saverange_source.bin", 0, (char*)"Hex", 0);
    EXPECT_GE(index, 0);

    unsigned char data[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
    WriteBytes(data, 0, 8);

    // Save a range to another file
    FileSaveRange("saverange_dest.bin", 2, 4);

    // Open destination file and verify
    int index2 = FileOpen("saverange_dest.bin", 0, (char*)"Hex", 0);
    EXPECT_GE(index2, 0);
    EXPECT_EQ(ReadUByte(0), 0x22u);
    EXPECT_EQ(ReadUByte(1), 0x33u);
    EXPECT_EQ(ReadUByte(2), 0x44u);
    EXPECT_EQ(ReadUByte(3), 0x55u);

    FileClose();

    // Clean up
    remove("saverange_source.bin");
    remove("saverange_dest.bin");
}

TEST(b010EditorTests, MakeDir) {
    // Test creating a directory
    MakeDir("test_mkdir_dir");

    // Verify directory exists by trying to create a file in it
    init_file_manager();
    int index = FileOpen("test_mkdir_dir/test_file.txt", 0, (char*)"Hex", 0);
    EXPECT_GE(index, 0);
    WriteUInt(0, 0x12345678);
    FileSave();
    FileClose();

    // Clean up
    remove("test_mkdir_dir/test_file.txt");
    _rmdir("test_mkdir_dir");
}

