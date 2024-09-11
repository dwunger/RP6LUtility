#include "b010_editor.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <windows.h>
#include <commdlg.h>

void init_emulator() {
    printf("Emulator Initialized");
}


// 010 editor's variants of GLIBC functions from String.h & stdio variants

int Strcmp(const char *str1, const char *str2) {
    printf("Strcmp function called with parameters:\n");
    printf("- str1: %s\n", str1);
    printf("- str2: %s\n", str2);
    return strcmp(str1, str2);
}

const char *Strstr(const char *str, const char *substr) {
    printf("Strstr function called with parameters:\n");
    printf("- str: %s\n", str);
    printf("- substr: %s\n", substr);
    return strstr(str, substr);
}

const char *SubStr(const char *str, int start, int len) {
    printf("SubStr function called with parameters:\n");
    printf("- str: %s\n", str);
    printf("- start: %d\n", start);
    printf("- len: %d\n", len);
    
    if (str == NULL) {
        return NULL;
    }

    size_t str_len = strlen(str);

    // Handle negative start index
    if (start < 0) {
        start = 0;
    }

    // Handle start beyond string length
    if ((size_t)start >= str_len) {
        char *empty = (char *)malloc(1);
        if (empty == NULL) {
            perror("Memory allocation failed");
            return NULL;
        }
        empty[0] = '\0';
        return empty;
    }

    // Handle negative len (extract to end of string)
    if (len < 0) {
        len = (int)(str_len - start);
    }

    // Adjust len if it goes beyond the end of the string
    if ((size_t)(start + len) > str_len) {
        len = (int)(str_len - start);
    }

    // Allocate memory for the substring
    char *substr = (char *)malloc((size_t)len + 1);
    if (substr == NULL) {
        perror("Memory allocation failed");
        return NULL;
    }

    // Copy the substring
    memcpy(substr, str + start, (size_t)len);
    substr[len] = '\0';  // Null-terminate the string

    return substr;
}


void FreeSubStr(const char* str) {
    free((void*)str);
}

size_t Strlen(const char *str) {

    if (!str) { perror("Strlen\n"); }

    const char *temp_ptr = str;

    while (*temp_ptr != '\0') {
        temp_ptr++;
    }

    return (size_t)(temp_ptr - str);
}

const char *SPrintf(const char *format, ...) {
    printf("SPrintf function called with format string: %s\n", format);
    // Add any necessary logic to handle the variable arguments and return a default value
    return "";
}

void InsertBytes(int offset, int size, int value) {
    printf("InsertBytes function called with parameters:\n");
    printf("- offset: %d\n", offset);
    printf("- size: %d\n", size);
    printf("- value: %d\n", value);
    // ...
}

int FindOpenFile(const char *path) {
    printf("FindOpenFile function called with path: %s\n", path);
    // ...
    return -1;
}

void FileSelect(int index) {
    printf("FileSelect function called with index: %d\n", index);
    // ...
}

int FileOpen(const char *filename, int runTemplate, const char *editAs, int openDuplicate) {
    printf("FileOpen function called with parameters:\n");
    printf("- path: %s\n", filename);
    printf("- readonly: %d\n", runTemplate);
    printf("- mode: %s\n", editAs);
    printf("- bigendian: %d\n", openDuplicate);
 
    if (filename == NULL) { 
        perror("FileOpen: unable to open file");
        return -1;
    }
    
    if (!runTemplate) {
        perror("Templates are not currently supported");
    }
    
    if (!openDuplicate) {
        perror("Fopen: Current implementation does not support \
                duplicate handles to open file");
    }
    
    if (strcmp(editAs, "Hex")) {
        perror("FileOpen: Currently only byte mode/Hex is supported");
    }
    FILE *file;
    (void)file;  // Silences the unused variable warning
    // ...
    return 0;
}

const char *FileNameGetBase(const char *path) {
    printf("FileNameGetBase function called with path: %s\n", path);
    // ...
    return "";
}

const char *FileNameGetPath(const char *path) {
    printf("FileNameGetPath function called with path: %s\n", path);
    // ...
    return "";
}

const char *ReadString(int offset) {
    printf("ReadString function called with offset: %d\n", offset);
    // ...
    return "";
}

void MakeDir(const char *path) {
    printf("MakeDir function called with path: %s\n", path);
    // ...
}

void FileSaveRange(const char *path, int offset, int size) {
    printf("FileSaveRange function called with parameters:\n");
    printf("- path: %s\n", path);
    printf("- offset: %d\n", offset);
    printf("- size: %d\n", size);
    // ...
}

int FindOpenFileW(const char *path) {
    printf("FindOpenFileW function called with path: %s\n", path);
    // ...
    return -1;
}

void WriteBytes(unsigned char *buffer, int offset, int size) {
    printf("WriteBytes function called with parameters:\n");
    printf("- offset: %d\n", offset);
    printf("- size: %d\n", size);
    // ...
}

const char *InputOpenFileName(const char *title, const char *filter, const char *default_ext) {
    printf("InputOpenFileName function called with parameters:\n");
    printf("- title: %s\n", title);
    printf("- filter: %s\n", filter);
    printf("- default_ext: %s\n", default_ext);
    static char path[MAX_PATH];
    OPENFILENAME  ofn;        
    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize     = sizeof(ofn);
    ofn.hwndOwner       = NULL;
    ofn.hInstance       = NULL;
    ofn.lpstrFilter     = "RPACK\0*.rpack\0\0";    
    ofn.lpstrFile       = path;
    ofn.nMaxFile        = MAX_PATH;
    ofn.lpstrTitle      = title;
    ofn.Flags           = OFN_NONETWORKBUTTON |
                          OFN_FILEMUSTEXIST |
                          OFN_HIDEREADONLY;
    if (!GetOpenFileName(&ofn)) {
        perror("InputOpenFileName: Invalid resource path supplied.\n");
        exit(EXIT_FAILURE);
    }
    printf("File selected:\n- %s\n", path);
    return path; 
}

void RunTemplate(const char *template_name) {
    printf("RunTemplate function called with template name: %s\n", template_name);
    // ... 
}

uint32_t ReadUInt(int offset) {
    printf("ReadUInt function called with offset: %d\n", offset);
    // Add any necessary logic or return a default value
    return 0;
}

uint16_t ReadUShort(int offset) {
    printf("ReadUShort function called with offset: %d\n", offset);
    // Add any necessary logic or return a default value
    return 0;
}

uint8_t ReadUByte(int offset) {
    printf("ReadUByte function called with offset: %d\n", offset);
    // Add any necessary logic or return a default value
    return 0;
}

void FileSave() {
    printf("FileSave function called\n");
    // Add any necessary logic
}

void FileClose() {
    printf("FileClose function called\n");
    // Add any necessary logic
}

void WriteUInt(int offset, uint32_t value) {
    printf("WriteUInt function called with parameters:\n");
    printf("- offset: %d\n", offset);
    printf("- value: %u\n", value);
    // Add any necessary logic
}


/**
 * 010 Editor Manual
 *
 * Function: FileOpen
 * Opens a file specified by the UTF-8 encoded string `filename`
 * into the editor.
 *
 * Parameters:
 *   - `filename` (const char): The name of the file to be opened.
 *   - `runTemplate` (int, optional): If true, executes a template
 *     associated with the file.
 *   - `editAs` (char[], optional): Specifies the "Edit As" mode for
 *     the opened file (e.g., "Hex", "Text", "Unicode").
 *   - `openDuplicate` (int, optional): If true, creates a duplicate
 *     copy of the file if it's already open; if false, no action is
 *     taken.
 *
 * Returns:
 *   - (int): The file index of the opened file if successful.
 *            - The file index of the already opened file if
 *              `openDuplicate` is false and the file is already open.
 *            - A negative number if the file could not be opened.
 *              No error message is displayed if the file is not found.
 *
 * Notes:
 * - Can be called in a Script or in a Template with special
 *   permission to read other files.
 * - To open a logical drive, use `FileOpen("Drive X:")`, where X is
 *   the drive letter.
 * - For physical drives, use `FileOpen("Physical Drive n:")`, where
 *   n is the drive number.
 * - Processes can be opened using `OpenProcessById` or
 *   `OpenProcessByName` functions.
 */
