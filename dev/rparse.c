#define DEBUG_RPACK_PATH1 "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Dying Light 2\\ph\\work\\data_platform\\pc\\assets\\common_textures_0_pc.rpack"

#define DEBUG_RPACK_PATH2 "X:\\SteamLibrary\\steamapps\\common\\Dying Light 2\\ph\\work\\data_platform\\pc\\assets\\common_textures_0_pc.rpack"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

long int get_file_size(FILE *file) {
    // Get current position in file
    long int position = ftell(file);
    if (position == -1) {
        perror("Invalid position");
        return -1;
    }
    // Jump to EOF
    fseek(file, 0L, SEEK_END);
    long int file_size = ftell(file);
    
    fseek(file, position, SEEK_SET);

    return file_size;
}

int main(int argc, char *argv[]) {
    char path[MAX_PATH];

    char debug_path[MAX_PATH] = DEBUG_RPACK_PATH2;
    //char debug_path[MAX_PATH] = ;
    /* Check for command line arguments */
    /*  Set default path for debugging  */
    if (argc > 1) {
        strcpy(path, argv[1]);
    } else {
        strcpy(path, debug_path);
    }
    printf(path);
    /* Loop over command line arguments*/
    for (int argument = 0; argument < argc; argument++) {
        //printf("%s\n", argv[argument]);
        //Do something...
    }
    
    FILE *file = fopen(path, "r");

    if (file == NULL) {
        perror("Unable to access file");
    }
    
    

    
    return 0;
}

