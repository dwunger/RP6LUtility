#include "b010_editor.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <windows.h>
#include <commdlg.h>


/* Internal */

#define MAX_PATH 260
#define MAX_OPEN_FILES 512

typedef struct {
    char path[MAX_PATH];
    bool is_open;
    char edit_as[20];
} FileEntry;

typedef struct {
    FileEntry files[MAX_OPEN_FILES];
    int current_file_index;
    int num_open_files;
} FileManager;

FileManager file_manager;


/* Internal */


/* Initializers */

void init_file_manager() {
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        file_manager.files[i].is_open = false;
        file_manager.files[i].path[0] = '\0';
        file_manager.files[i].edit_as[0] = '\0';
    }
    file_manager.current_file_index = -1;
    file_manager.num_open_files = 0;
    DEBUG_LOG("File Manager Initialized\n");
}

// 010 editor's variants of GLIBC functions from String.h & stdio variants

int Strcmp(const char *str1, const char *str2) {
    DEBUG_LOG("Strcmp function called with parameters:\n");
    DEBUG_LOG("- str1: %s\n", str1);
    DEBUG_LOG("- str2: %s\n", str2);
    return strcmp(str1, str2);
}

const char *Strstr(const char *str, const char *substr) {
    DEBUG_LOG("Strstr function called with parameters:\n");
    DEBUG_LOG("- str: %s\n", str);
    DEBUG_LOG("- substr: %s\n", substr);
    return strstr(str, substr);
}

const char *SubStr(const char *str, int start, int len) {
    DEBUG_LOG("SubStr function called with parameters:\n");
    DEBUG_LOG("- str: %s\n", str);
    DEBUG_LOG("- start: %d\n", start);
    DEBUG_LOG("- len: %d\n", len);
    
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
    DEBUG_LOG("SPrintf function called with format string: %s\n", format);
    // Add any necessary logic to handle the variable arguments and return a default value
    (void)format;
    return "";
}

void InsertBytes(int offset, int size, int value) {
    DEBUG_LOG("InsertBytes function called with parameters:\n");
    DEBUG_LOG("- offset: %d\n", offset);
    DEBUG_LOG("- size: %d\n", size);
    DEBUG_LOG("- value: %d\n", value);
    // ...
    (void)offset;
    (void)size;
    (void)value;
}

int FindOpenFile(const char *path) {
    DEBUG_LOG("FindOpenFile function called with path: %s\n", path);
    
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (file_manager.files[i].is_open && strcmp(file_manager.files[i].path, path) == 0) {
            return i;
        }
    }
    
    return -1;
}

void FileSelect(int index) {
    if (index >= 0 && index < MAX_OPEN_FILES && file_manager.files[index].is_open) {
        file_manager.current_file_index = index;
        DEBUG_LOG("Selected file: %s (index: %d)\n", file_manager.files[index].path, index);
    } else {
        printf("Error: Invalid file index\n");
    }
}

int FileOpen(const char filename[], int runTemplate, char editAs[], int openDuplicate) {
    DEBUG_LOG("FileOpen function called with parameters:\n");
    DEBUG_LOG("- filename: %s\n", filename);
    DEBUG_LOG("- runTemplate: %d\n", runTemplate);
    DEBUG_LOG("- editAs: %s\n", editAs);
    DEBUG_LOG("- openDuplicate: %d\n", openDuplicate);

    if (filename == NULL) {
        printf("FileOpen: unable to open file (null filename)\n");
        return -1;
    }

    int index = FindOpenFile(filename);
    if (index != -1 && !openDuplicate) {
        DEBUG_LOG("File already open: %s (index: %d)\n", filename, index);
        return index;
    }

    // Find the first available slot
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (!file_manager.files[i].is_open) {
            strncpy(file_manager.files[i].path, filename, MAX_PATH - 1);
            file_manager.files[i].path[MAX_PATH - 1] = '\0';
            file_manager.files[i].is_open = true;
            
            if (editAs[0] != '\0') {
                strncpy(file_manager.files[i].edit_as, editAs, 19);
                file_manager.files[i].edit_as[19] = '\0';
            } else {
                strcpy(file_manager.files[i].edit_as, "Hex");  // Default to Hex
            }

            file_manager.num_open_files++;
            file_manager.current_file_index = i;

            DEBUG_LOG("File opened: %s (index: %d, editAs: %s)\n", filename, i, file_manager.files[i].edit_as);

            if (runTemplate) {
                printf("Warning: Templates are not currently supported\n");
            }

            return i;
        }
    }

    printf("Error: Maximum number of open files reached\n");
    return -1;
}

const char *FileNameGetBase(const char *path) {
    DEBUG_LOG("FileNameGetBase function called with path: %s\n", path);
    // ...
    (void)path;
    return "";
}

const char *FileNameGetPath(const char *path) {
    DEBUG_LOG("FileNameGetPath function called with path: %s\n", path);
    // ...

    (void)path;
    return "";
}

const char *ReadString(int offset) {
    DEBUG_LOG("ReadString function called with offset: %d\n", offset);
    // ...
    (void)offset;
    return "";
}

void MakeDir(const char *path) {
    DEBUG_LOG("MakeDir function called with path: %s\n", path);
    // ...
    (void)path;
}

void FileSaveRange(const char *path, int offset, int size) {
    DEBUG_LOG("FileSaveRange function called with parameters:\n");
    DEBUG_LOG("- path: %s\n", path);
    DEBUG_LOG("- offset: %d\n", offset);
    DEBUG_LOG("- size: %d\n", size);
    // ...
    (void)path;
    (void)offset;
    (void)size;
}

int FindOpenFileW(const char *path) {
    DEBUG_LOG("FindOpenFileW function called with path: %s\n", path);
    // ...
    (void)path;
    return -1;
}

void WriteBytes(unsigned char *buffer, int offset, int size) {
    DEBUG_LOG("WriteBytes function called with parameters:\n");
    DEBUG_LOG("- offset: %d\n", offset);
    DEBUG_LOG("- size: %d\n", size);
    // ...
    (void)buffer;
    (void)offset;
    (void)size;
}

const char *InputOpenFileName(const char *title, const char *filter, const char *default_ext) {
    DEBUG_LOG("InputOpenFileName function called with parameters:\n");
    DEBUG_LOG("- title: %s\n", title);
    DEBUG_LOG("- filter: %s\n", filter);
    DEBUG_LOG("- default_ext: %s\n", default_ext);
    static char path[MAX_PATH];
    OPENFILENAME  ofn;        
    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize     = sizeof(ofn);
    ofn.hwndOwner       = NULL;
    ofn.hInstance       = NULL;
    ofn.lpstrFilter     = "RPACK\0*.rpack\0\0"; // Filter should be used here   
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
    DEBUG_LOG("InputOpenFileName: File selected:\n- %s\n", path);
    return path; 
}

void RunTemplate(const char *template_name) {
    DEBUG_LOG("RunTemplate function called with template name: %s\n", template_name);
    (void)template_name;
    // ... 
}

uint32_t ReadUInt(int offset) {
    DEBUG_LOG("ReadUInt function called with offset: %d\n", offset);
    // Add any necessary logic or return a default value
    (void)offset;
    return 0;
}

uint16_t ReadUShort(int offset) {
    DEBUG_LOG("ReadUShort function called with offset: %d\n", offset);
    // Add any necessary logic or return a default value
    (void)offset;
    return 0;
}

uint8_t ReadUByte(int offset) {
    DEBUG_LOG("ReadUByte function called with offset: %d\n", offset);
    // Add any necessary logic or return a default value
    (void)offset;
    return 0;
}

void FileSave() {
    DEBUG_LOG("FileSave function called\n");
    // Add any necessary logic
}

void FileClose() {
    DEBUG_LOG("FileClose function called\n");
    // Add any necessary logic
}

void WriteUInt(int offset, uint32_t value) {
    DEBUG_LOG("WriteUInt function called with parameters:\n");
    DEBUG_LOG("- offset: %d\n", offset);
    DEBUG_LOG("- value: %u\n", value);
    // Add any necessary logic
    (void)offset;
    (void)value;
}

void init_emulator() {
    DEBUG_LOG("Emulator Initialized");
    file_manager.current_file_index = -1;
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
