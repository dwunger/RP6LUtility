#include <stdint.h>
#include <stddef.h>

#ifndef EDITOR_H
#define EDITOR_H

#ifdef DEBUG_PRINT
    #define DEBUG_LOG(...) printf(__VA_ARGS__)
#else
    #define DEBUG_LOG(...) ((void)0)
#endif

// Initialize the emulator
void init_emulator();

// 010 editor's variants of GLIBC functions from String.h & stdio variants

/**
 * @brief Compare two strings.
 * @param str1 The first string.
 * @param str2 The second string.
 * @return An integer less than, equal to, or greater than zero if str1 is found,
 *         respectively, to be less than, to match, or be greater than str2.
 */
int Strcmp(const char *str1, const char *str2);

/**
 * @brief Locate substring.
 * @param str The string to be scanned.
 * @param substr The substring to search for.
 * @return A pointer to the first occurrence of substr in str, or NULL if not found.
 */
const char *Strstr(const char *str, const char *substr);

/**
 * @brief Extract substring from string.
 * @param str The source string.
 * @param start The start index.
 * @param len The length of the substring.
 * @return The extracted substring.
 */
const char *SubStr(const char *str, int start, int len);

/**
 * @brief Get string length.
 * @param str The string.
 * @return The length of the string.
 */
size_t Strlen(const char *str);

/**
 * @brief Write formatted data to string.
 * @param format The format string.
 * @param ... Additional arguments.
 * @return The formatted string.
 */
const char *SPrintf(const char *format, ...);

/**
 * @brief Insert bytes into the current file.
 * @param offset The offset where to insert the bytes.
 * @param size The number of bytes to insert.
 * @param value The value to fill the inserted bytes with.
 */
void InsertBytes(int64_t offset, int64_t size, int value);


/**
 * @brief Searches through the list of all open files to see if the file indicated by the path is currently open.
 * 
 * This function checks whether a file specified by its path is currently open. If the file is open, it returns the index 
 * of the file, which can be used with the FileSelect function. If the file is not found in the list of open files, 
 * the function returns -1.
 * 
 * @param path The path of the file to check.
 * @return The index of the open file if found, or -1 if the file is not found.
 */
int FindOpenFile(const char *path);

/**
 * @brief Select a file to be the current active file.
 * @param index The index of the file to select.
 */
void FileSelect(int index);

/**
 * @brief Open a file in the editor.
 * @param filename The name of the file to open.
 * @param runTemplate If true, run the associated template on the file.
 * @param editAs The mode to edit the file in (e.g., "Hex", "Text", "Unicode").
 * @param openDuplicate If true, open a duplicate copy if the file is already open.
 * @return The file index of the opened file, or a negative number if the file couldn't be opened.
 */
int FileOpen(const char *filename, int runTemplate, char *editAs, int openDuplicate);

/**
 * @brief Get the base name of a file path.
 * @param path The full path of the file.
 * @return The base name of the file.
 */
const char *FileNameGetBase(const char *path);

/**
 * @brief Get the path portion of a full file name.
 * @param path The full path of the file.
 * @return The path portion of the file name.
 */
const char *FileNameGetPath(const char *path);

/**
 * @brief Read a string from the current file.
 * @param offset The offset to start reading from.
 * @return The string read from the file.
 */
const char *ReadString(int offset);

/**
 * @brief Create a directory.
 * @param path The path of the directory to create.
 */
void MakeDir(const char *path);

/**
 * @brief Save a range of bytes from the current file to a new file.
 * @param path The path of the file to save to.
 * @param offset The starting offset of the range to save.
 * @param size The number of bytes to save.
 */
void FileSaveRange(const char *path, int offset, int size);

/**
 * @brief Search for an open file by its path (wide character version).
 * @param path The path of the file to find.
 * @return The index of the file if found, or -1 if not found.
 */
int FindOpenFileW(const char *path);

/**
 * @brief Write bytes to the current file.
 * @param buffer The buffer containing the bytes to write.
 * @param offset The offset where to start writing.
 * @param size The number of bytes to write.
 */
void WriteBytes(unsigned char *buffer, int64_t offset, int64_t size);

/**
 * @brief Show a file open dialog and return the selected file name.
 * @param title The title of the dialog.
 * @param filter The file type filter.
 * @param default_ext The default file extension.
 * @return The full path of the chosen file, or an empty string if cancelled.
 */
const char *InputOpenFileName(const char *title, const char *filter, const char *default_ext);

/**
 * @brief Run a template on the current file.
 * @param template_name The name of the template to run.
 */
void RunTemplate(const char *template_name);


/**
 * @brief Read an unsigned 32-bit integer from the current file.
 * @param offset The offset to read from.
 * @return The unsigned 32-bit integer read from the file.
 */
uint32_t ReadUInt(int offset);

/**
 * @brief Read an unsigned 16-bit integer from the current file.
 * @param offset The offset to read from.
 * @return The unsigned 16-bit integer read from the file.
 */
uint16_t ReadUShort(int offset);

/**
 * @brief Read an unsigned 8-bit integer from the current file.
 * @param offset The offset to read from.
 * @return The unsigned 8-bit integer read from the file.
 */
uint8_t ReadUByte(int offset);

/**
 * @brief Save the current file.
 */
void FileSave();

/**
 * @brief Close the current file.
 */
void FileClose();

/**
 * @brief Write an unsigned 32-bit integer to the current file.
 * @param offset The offset to write to.
 * @param value The value to write.
 */
void WriteUInt(int offset, uint32_t value);

/**
 * @brief Get the current file index.
 * @return The index of the currently selected file, or -1 if no file is selected.
 */
int GetFileNum(void);

/**
 * @brief Get the size of the current file.
 * @return The size in bytes of the current file.
 */
size_t FileSize(void);

/**
 * @brief Check if a file exists.
 * @param path The path of the file to check.
 * @return 1 if the file exists, 0 otherwise.
 */
int FileExists(const char *path);

/**
 * @brief Read bytes from the current file into a buffer.
 * @param buffer The buffer to read into.
 * @param offset The offset to start reading from.
 * @param size The number of bytes to read.
 */
void ReadBytes(unsigned char *buffer, int offset, int size);

/**
 * @brief Write an unsigned 16-bit integer to the current file.
 * @param offset The offset to write to.
 * @param value The value to write.
 */
void WriteUShort(int offset, uint16_t value);

/**
 * @brief Write an unsigned 8-bit integer to the current file.
 * @param offset The offset to write to.
 * @param value The value to write.
 */
void WriteUByte(int offset, uint8_t value);

/**
 * @brief Write a string to the current file.
 * @param offset The offset to write to.
 * @param str The string to write.
 */
void WriteString(int64_t offset, const char *str);

/**
 * @brief Delete bytes from the current file.
 * @param offset The offset to start deleting from.
 * @param size The number of bytes to delete.
 */
void DeleteBytes(int64_t offset, int64_t size);

/**
 * @brief Read an unsigned 64-bit integer from the current file.
 * @param offset The offset to read from.
 * @return The unsigned 64-bit integer read from the file.
 */
uint64_t ReadUInt64(int offset);

/**
 * @brief Write an unsigned 64-bit integer to the current file.
 * @param offset The offset to write to.
 * @param value The value to write.
 */
void WriteUInt64(int offset, uint64_t value);

/**
 * @brief Show a directory selection dialog.
 * @param title The title of the dialog.
 * @param default_path The default path to start from.
 * @return The selected directory path, or empty string if cancelled.
 */
const char *InputDirectory(const char *title, const char *default_path);

/**
 * @brief Find first occurrence of a pattern in the current file.
 * @param pattern The pattern to search for.
 * @param matchCase Case sensitive search.
 * @param matchWholeWord Match whole words only.
 * @param wildcardMatchSingleChar Wildcard for single char.
 * @param wildcardMatchMultipleChar Wildcard for multiple chars.
 * @param method Search method (1 = ASCII string).
 * @param start Start offset for search.
 * @param size Size of region to search.
 * @return Offset of first match, or -1 if not found.
 */
int64_t FindFirst(const char *pattern, int matchCase, int matchWholeWord,
                  int wildcardMatchSingleChar, float wildcardMatchMultipleChar,
                  int method, int64_t start, int64_t size);

/* File list for FindFiles */
typedef struct {
    char filename[260];
} TFileEntry;

typedef struct {
    int filecount;
    TFileEntry *file;
} TFileList;

/**
 * @brief Find files matching a pattern in a directory.
 * @param dir The directory to search.
 * @param pattern The file pattern (e.g. "*.desc").
 * @return A TFileList structure containing matching files.
 */
TFileList FindFiles(const char *dir, const char *pattern);

/**
 * @brief Free a TFileList structure.
 * @param fl The file list to free.
 */
void FreeFindFiles(TFileList *fl);

/* Internals */

void init_file_manager();



#endif // EDITOR_H
