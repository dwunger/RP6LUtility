#include "010editor.h"
#include <stdio.h>

void init_emulator() {
    printf("Emulator Initialized");
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

int FileOpen(
        const char *filename,
        const int runTemplate, 
        const char *editAs,
        const int openDuplicate
        ) {

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
    
    


}



