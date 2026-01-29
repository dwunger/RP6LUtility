#include "b010_editor.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <windows.h>
#include <commdlg.h>
#include <stdarg.h>  // for SPrintf

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#define MAX_OPEN_FILES 512

typedef struct {
    char path[MAX_PATH];
    bool is_open;
    char edit_as[20];
    unsigned char *buffer;      // in‑memory file content
    size_t buffer_size;         // current number of bytes
    size_t buffer_capacity;     // allocated size of buffer
} FileEntry;

typedef struct {
    FileEntry files[MAX_OPEN_FILES];
    int current_file_index;
    int num_open_files;
} FileManager;

static FileManager file_manager;

/* Internal utility functions */

// Normalize path separators to forward slashes (in-place).
static void normalize_path(char *path) {
    if (!path) return;
    for (char *p = path; *p; p++) {
        if (*p == '\\') *p = '/';
    }
}

// Return pointer to the base file name from a given path.
static const char *get_basename(const char *path) {
    if (!path) return "";
    const char *base1 = strrchr(path, '\\');
    const char *base2 = strrchr(path, '/');
    const char *base = (base1 > base2) ? base1 : base2;
    return base ? base + 1 : path;
}

// Return directory portion of path into a static buffer.
static const char *get_dirname(const char *path) {
    static char dir[MAX_PATH];
    if (!path) {
        strcpy(dir, "");
        return dir;
    }
    const char *sep = strrchr(path, '\\');
    if (!sep) {
        sep = strrchr(path, '/');
    }
    if (sep) {
        size_t len = sep - path;
        if (len >= MAX_PATH) {
            len = MAX_PATH - 1;
        }
        strncpy(dir, path, len);
        dir[len] = '\0';
    } else {
        // No directory separator found: return "."
        strcpy(dir, ".");
    }
    return dir;
}

/* Initializers */

void init_file_manager() {
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        file_manager.files[i].is_open = false;
        file_manager.files[i].path[0] = '\0';
        file_manager.files[i].edit_as[0] = '\0';
        file_manager.files[i].buffer = NULL;
        file_manager.files[i].buffer_size = 0;
        file_manager.files[i].buffer_capacity = 0;
    }
    file_manager.current_file_index = -1;
    file_manager.num_open_files = 0;
    DEBUG_LOG("File Manager Initialized\n");
}

/* 010 Editor's variants of GLIBC functions */

// Compare two strings.
int Strcmp(const char *str1, const char *str2) {
    DEBUG_LOG("Strcmp called:\n  str1: %s\n  str2: %s\n", str1, str2);
    return strcmp(str1, str2);
}

// Locate substring.
const char *Strstr(const char *str, const char *substr) {
    DEBUG_LOG("Strstr called:\n  str: %s\n  substr: %s\n", str, substr);
    return strstr(str, substr);
}

// Extract substring. Caller is responsible for freeing the returned memory.
const char *SubStr(const char *str, int start, int len) {
    DEBUG_LOG("SubStr called:\n  str: %s\n  start: %d\n  len: %d\n", str, start, len);
    
    if (str == NULL) {
        return NULL;
    }
    size_t str_len = strlen(str);
    if (start < 0) {
        start = 0;
    }
    if ((size_t)start >= str_len) {
        char *empty = malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
    if (len < 0) {
        len = (int)(str_len - start);
    }
    if ((size_t)(start + len) > str_len) {
        len = (int)(str_len - start);
    }
    char *substr = malloc((size_t)len + 1);
    if (!substr) {
        perror("Memory allocation failed");
        return NULL;
    }
    memcpy(substr, str + start, (size_t)len);
    substr[len] = '\0';
    return substr;
}

// Free memory allocated by SubStr.
void FreeSubStr(const char* str) {
    free((void*)str);
}

// Get string length.
size_t Strlen(const char *str) {
    if (!str) { perror("Strlen: NULL pointer"); return 0; }
    const char *temp_ptr = str;
    while (*temp_ptr != '\0') {
        temp_ptr++;
    }
    return (size_t)(temp_ptr - str);
}

// Write formatted data to string.
// The caller is responsible for freeing the returned memory.
const char *SPrintf(const char *format, ...) {
    DEBUG_LOG("SPrintf called with format: %s\n", format);
    va_list args;
    va_start(args, format);
    int len = vsnprintf(NULL, 0, format, args);
    va_end(args);
    if (len < 0) return "";
    char *result = malloc((size_t)len + 1);
    if (!result) return "";
    va_start(args, format);
    vsnprintf(result, (size_t)len + 1, format, args);
    va_end(args);
    return result;
}

// Insert bytes into the current file.
void InsertBytes(int offset, int size, int value) {
    DEBUG_LOG("InsertBytes called:\n  offset: %d\n  size: %d\n  value: %d\n", offset, size, value);
    if (file_manager.current_file_index < 0) {
        printf("InsertBytes: No file is currently open\n");
        return;
    }
    FileEntry *file = &file_manager.files[file_manager.current_file_index];
    if (offset < 0 || offset > (int)file->buffer_size) {
        printf("InsertBytes: Invalid offset\n");
        return;
    }
    size_t new_size = file->buffer_size + size;
    if (new_size > file->buffer_capacity) {
        size_t new_capacity = file->buffer_capacity > 0 ? file->buffer_capacity * 2 : 256;
        while (new_capacity < new_size)
            new_capacity *= 2;
        unsigned char *new_buffer = realloc(file->buffer, new_capacity);
        if (!new_buffer) {
            perror("InsertBytes: Memory allocation failed");
            return;
        }
        file->buffer = new_buffer;
        file->buffer_capacity = new_capacity;
    }
    // Move existing data after offset to make room.
    memmove(file->buffer + offset + size, file->buffer + offset, file->buffer_size - offset);
    // Fill inserted region with the specified value.
    memset(file->buffer + offset, (unsigned char)value, size);
    file->buffer_size = new_size;
}

// Searches for an open file by its path.
int FindOpenFile(const char *path) {
    DEBUG_LOG("FindOpenFile called with path: %s\n", path);
    char normalized[MAX_PATH];
    strncpy(normalized, path, MAX_PATH - 1);
    normalized[MAX_PATH - 1] = '\0';
    normalize_path(normalized);
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (file_manager.files[i].is_open && strcmp(file_manager.files[i].path, normalized) == 0) {
            return i;
        }
    }
    return -1;
}

// Select a file to be the current active file.
void FileSelect(int index) {
    if (index >= 0 && index < MAX_OPEN_FILES && file_manager.files[index].is_open) {
        file_manager.current_file_index = index;
        DEBUG_LOG("FileSelect: Selected file %s (index %d)\n", file_manager.files[index].path, index);
    } else {
        printf("Error: Invalid file index\n");
    }
}

// Open a file in the editor.
int FileOpen(const char filename[], int runTemplate, char editAs[], int openDuplicate) {
    DEBUG_LOG("FileOpen called:\n  filename: %s\n  runTemplate: %d\n  editAs: %s\n  openDuplicate: %d\n",
              filename, runTemplate, editAs, openDuplicate);
              
    if (filename == NULL) {
        printf("FileOpen: unable to open file (null filename)\n");
        return -1;
    }
    
    int index = FindOpenFile(filename);
    if (index != -1 && !openDuplicate) {
        DEBUG_LOG("FileOpen: File already open (%s) at index %d\n", filename, index);
        return index;
    }
    
    // Find first available slot.
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (!file_manager.files[i].is_open) {
            strncpy(file_manager.files[i].path, filename, MAX_PATH - 1);
            file_manager.files[i].path[MAX_PATH - 1] = '\0';
            normalize_path(file_manager.files[i].path);
            file_manager.files[i].is_open = true;
            if (editAs[0] != '\0') {
                strncpy(file_manager.files[i].edit_as, editAs, sizeof(file_manager.files[i].edit_as) - 1);
                file_manager.files[i].edit_as[sizeof(file_manager.files[i].edit_as) - 1] = '\0';
            } else {
                strcpy(file_manager.files[i].edit_as, "Hex");  // default mode
            }
            file_manager.num_open_files++;
            file_manager.current_file_index = i;
            
            // Try to load file content from disk.
            FILE *fp = fopen(filename, "rb");
            if (fp) {
                fseek(fp, 0, SEEK_END);
                long fsize = ftell(fp);
                rewind(fp);
                if (fsize > 0) {
                    file_manager.files[i].buffer = malloc(fsize);
                    if (file_manager.files[i].buffer) {
                        size_t read_bytes = fread(file_manager.files[i].buffer, 1, fsize, fp);
                        file_manager.files[i].buffer_size = read_bytes;
                        file_manager.files[i].buffer_capacity = read_bytes;
                    }
                } else {
                    // Empty file: allocate a small initial buffer.
                    file_manager.files[i].buffer = malloc(256);
                    if (file_manager.files[i].buffer) {
                        file_manager.files[i].buffer_size = 0;
                        file_manager.files[i].buffer_capacity = 256;
                    }
                }
                fclose(fp);
            } else {
                // File doesn't exist: allocate an initial buffer.
                file_manager.files[i].buffer = malloc(256);
                if (file_manager.files[i].buffer) {
                    file_manager.files[i].buffer_size = 0;
                    file_manager.files[i].buffer_capacity = 256;
                }
            }
            
            DEBUG_LOG("FileOpen: Opened file %s (index %d, editAs: %s)\n", 
                      filename, i, file_manager.files[i].edit_as);
            
            if (runTemplate) {
                printf("Warning: Templates are not currently supported\n");
            }
            
            return i;
        }
    }
    printf("Error: Maximum number of open files reached\n");
    return -1;
}

// Get the base name of a file path.
const char *FileNameGetBase(const char *path) {
    DEBUG_LOG("FileNameGetBase called with path: %s\n", path);
    return get_basename(path);
}

// Get the path portion of a full file name.
const char *FileNameGetPath(const char *path) {
    DEBUG_LOG("FileNameGetPath called with path: %s\n", path);
    return get_dirname(path);
}

// Read a string from the current file.
// Returns a dynamically allocated string which the caller should free.
const char *ReadString(int offset) {
    DEBUG_LOG("ReadString called with offset: %d\n", offset);
    if (file_manager.current_file_index < 0) {
        printf("ReadString: No file is currently open\n");
        return strdup("");
    }
    FileEntry *file = &file_manager.files[file_manager.current_file_index];
    if (offset < 0 || offset >= (int)file->buffer_size) {
        return strdup("");
    }
    // Find null terminator from offset.
    int len = 0;
    while (offset + len < (int)file->buffer_size && file->buffer[offset + len] != '\0') {
        len++;
    }
    char *str = malloc(len + 1);
    if (!str) {
        perror("ReadString: Memory allocation failed");
        return strdup("");
    }
    memcpy(str, file->buffer + offset, len);
    str[len] = '\0';
    return str;
}

// Create a directory.
void MakeDir(const char *path) {
    DEBUG_LOG("MakeDir called with path: %s\n", path);
    if (!CreateDirectoryA(path, NULL)) {
        DWORD err = GetLastError();
        if (err != ERROR_ALREADY_EXISTS) {
            printf("MakeDir: Failed to create directory %s (Error %lu)\n", path, err);
        }
    }
}

// Save a range of bytes from the current file to a new file.
void FileSaveRange(const char *path, int offset, int size) {
    DEBUG_LOG("FileSaveRange called:\n  path: %s\n  offset: %d\n  size: %d\n", path, offset, size);
    if (file_manager.current_file_index < 0) {
        printf("FileSaveRange: No file is currently open\n");
        return;
    }
    FileEntry *file = &file_manager.files[file_manager.current_file_index];
    if (offset < 0 || offset + size > (int)file->buffer_size) {
        printf("FileSaveRange: Invalid range\n");
        return;
    }
    FILE *fp = fopen(path, "wb");
    if (!fp) {
        perror("FileSaveRange: Failed to open file for writing");
        return;
    }
    size_t written = fwrite(file->buffer + offset, 1, size, fp);
    if (written != (size_t)size) {
        perror("FileSaveRange: Failed to write complete data");
    }
    fclose(fp);
}

// Searches for an open file by its path (wide character version).
// For simplicity, we treat it the same as FindOpenFile.
int FindOpenFileW(const char *path) {
    DEBUG_LOG("FindOpenFileW called with path: %s\n", path);
    return FindOpenFile(path);
}

// Write bytes to the current file.
void WriteBytes(unsigned char *buffer, int offset, int size) {
    DEBUG_LOG("WriteBytes called:\n  offset: %d\n  size: %d\n", offset, size);
    if (file_manager.current_file_index < 0) {
        printf("WriteBytes: No file is currently open\n");
        return;
    }
    FileEntry *file = &file_manager.files[file_manager.current_file_index];
    if (offset < 0) {
        printf("WriteBytes: Invalid offset\n");
        return;
    }
    int required_size = offset + size;
    if (required_size > (int)file->buffer_size) {
        if (required_size > (int)file->buffer_capacity) {
            size_t new_capacity = file->buffer_capacity > 0 ? file->buffer_capacity * 2 : 256;
            while (new_capacity < (size_t)required_size)
                new_capacity *= 2;
            unsigned char *new_buffer = realloc(file->buffer, new_capacity);
            if (!new_buffer) {
                perror("WriteBytes: Memory allocation failed");
                return;
            }
            file->buffer = new_buffer;
            file->buffer_capacity = new_capacity;
        }
        // Fill gap with zeros if necessary.
        if (offset > (int)file->buffer_size) {
            memset(file->buffer + file->buffer_size, 0, offset - file->buffer_size);
        }
        file->buffer_size = required_size;
    }
    memcpy(file->buffer + offset, buffer, size);
}

// Read an unsigned 32-bit integer from the current file.
uint32_t ReadUInt(int offset) {
    DEBUG_LOG("ReadUInt called with offset: %d\n", offset);
    if (file_manager.current_file_index < 0) {
        printf("ReadUInt: No file is currently open\n");
        return 0;
    }
    FileEntry *file = &file_manager.files[file_manager.current_file_index];
    if (offset < 0 || offset + (int)sizeof(uint32_t) > (int)file->buffer_size) {
        printf("ReadUInt: Invalid range\n");
        return 0;
    }
    uint32_t value;
    memcpy(&value, file->buffer + offset, sizeof(uint32_t));
    return value;
}

// Read an unsigned 16-bit integer from the current file.
uint16_t ReadUShort(int offset) {
    DEBUG_LOG("ReadUShort called with offset: %d\n", offset);
    if (file_manager.current_file_index < 0) {
        printf("ReadUShort: No file is currently open\n");
        return 0;
    }
    FileEntry *file = &file_manager.files[file_manager.current_file_index];
    if (offset < 0 || offset + (int)sizeof(uint16_t) > (int)file->buffer_size) {
        printf("ReadUShort: Invalid range\n");
        return 0;
    }
    uint16_t value;
    memcpy(&value, file->buffer + offset, sizeof(uint16_t));
    return value;
}

// Read an unsigned 8-bit integer from the current file.
uint8_t ReadUByte(int offset) {
    DEBUG_LOG("ReadUByte called with offset: %d\n", offset);
    if (file_manager.current_file_index < 0) {
        printf("ReadUByte: No file is currently open\n");
        return 0;
    }
    FileEntry *file = &file_manager.files[file_manager.current_file_index];
    if (offset < 0 || offset >= (int)file->buffer_size) {
        printf("ReadUByte: Invalid range\n");
        return 0;
    }
    return file->buffer[offset];
}

// Save the current file.
void FileSave() {
    DEBUG_LOG("FileSave called\n");
    if (file_manager.current_file_index < 0) {
        printf("FileSave: No file is currently open\n");
        return;
    }
    FileEntry *file = &file_manager.files[file_manager.current_file_index];
    FILE *fp = fopen(file->path, "wb");
    if (!fp) {
        perror("FileSave: Failed to open file for writing");
        return;
    }
    size_t written = fwrite(file->buffer, 1, file->buffer_size, fp);
    if (written != file->buffer_size) {
        perror("FileSave: Failed to write complete data");
    }
    fclose(fp);
}

// Close the current file.
void FileClose() {
    DEBUG_LOG("FileClose called\n");
    int idx = file_manager.current_file_index;
    if (idx >= 0 && idx < MAX_OPEN_FILES && file_manager.files[idx].is_open) {
        if (file_manager.files[idx].buffer) {
            free(file_manager.files[idx].buffer);
            file_manager.files[idx].buffer = NULL;
        }
        file_manager.files[idx].is_open = false;
        DEBUG_LOG("FileClose: Closed file %s (index %d)\n", file_manager.files[idx].path, idx);
        file_manager.num_open_files--;
        file_manager.current_file_index = -1;
    } else {
        printf("FileClose: No file is currently open\n");
    }
}

// Get the current file index.
int GetFileNum(void) {
    return file_manager.current_file_index;
}

// Write an unsigned 32-bit integer to the current file.
void WriteUInt(int offset, uint32_t value) {
    DEBUG_LOG("WriteUInt called:\n  offset: %d\n  value: %u\n", offset, value);
    if (file_manager.current_file_index < 0) {
        printf("WriteUInt: No file is currently open\n");
        return;
    }
    FileEntry *file = &file_manager.files[file_manager.current_file_index];
    int required_size = offset + sizeof(uint32_t);
    if (required_size > (int)file->buffer_size) {
        if (required_size > (int)file->buffer_capacity) {
            size_t new_capacity = file->buffer_capacity > 0 ? file->buffer_capacity * 2 : 256;
            while (new_capacity < (size_t)required_size)
                new_capacity *= 2;
            unsigned char *new_buffer = realloc(file->buffer, new_capacity);
            if (!new_buffer) {
                perror("WriteUInt: Memory allocation failed");
                return;
            }
            file->buffer = new_buffer;
            file->buffer_capacity = new_capacity;
        }
        if (offset > (int)file->buffer_size) {
            memset(file->buffer + file->buffer_size, 0, offset - file->buffer_size);
        }
        file->buffer_size = required_size;
    }
    memcpy(file->buffer + offset, &value, sizeof(uint32_t));
}

// Run a template on the current file. (Not implemented)
void RunTemplate(const char *template_name) {
    DEBUG_LOG("RunTemplate called with template name: %s\n", template_name);
    // Template functionality excluded.
}

const char *InputOpenFileName(const char *title, const char *filter, const char *default_ext) {
    DEBUG_LOG("InputOpenFileName called:\n  title: %s\n  filter: %s\n  default_ext: %s\n", title, filter, default_ext);
    static char path[MAX_PATH] = {0};
    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;  // Replace with a valid window handle if available.
    ofn.lpstrFile = path;
    path[0] = '\0';        // Clear any previous value.
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter = filter ? filter : "All Files\0*.*\0";
    ofn.lpstrTitle = title;
    ofn.lpstrDefExt = default_ext; // Optional: set the default extension.
    ofn.Flags = OFN_NONETWORKBUTTON | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    
    if (!GetOpenFileName(&ofn)) {
        DWORD err = CommDlgExtendedError();
        if (err != 0) {
            fprintf(stderr, "InputOpenFileName: GetOpenFileName failed with error %lu\n", err);
            exit(EXIT_FAILURE);
        }
        return NULL; // User cancelled.
    }
    DEBUG_LOG("InputOpenFileName: File selected: %s\n", path);
    return path;
}

// Initialize the emulator.
void init_emulator() {
    DEBUG_LOG("Emulator Initialized\n");
    file_manager.current_file_index = -1;
    init_file_manager();
}

/*
 * 010 Editor Manual (adapted):
 *
 * FileOpen:
 *   - Opens a file specified by a UTF-8 encoded string.
 *   - Parameters:
 *       filename: name of the file.
 *       runTemplate: if non-zero, run an associated template.
 *       editAs: specifies editing mode (e.g., "Hex", "Text", "Unicode").
 *       openDuplicate: if zero and the file is already open, return its index.
 *   - Returns: file index if successful, negative value if failed.
 *
 * (Other functions follow similar patterns.)
 */
