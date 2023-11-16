#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

int main(int argc, char *argv[]) {
    char path[MAX_PATH];

    char debug_path[MAX_PATH] = "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Dying Light 2\\ph\\work\\data_platform\\pc\\assets\\common_textures_0_pc.rpack"
    //char debug_path[MAX_PATH] = "X:\\SteamLibrary\\steamapps\\common\\Dying Light 2\\ph\\work\\data_platform\\pc\\assets\\common_textures_0_pc.rpack";
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
    

    return 0;
}

