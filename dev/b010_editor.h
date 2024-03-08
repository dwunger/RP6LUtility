#ifndef EDITOR_H
#define EDITOR_H

void init_emulator();
int FileOpen(
    const char *filename,
    int runTemplate,
    const char *editAs, 
    int openDuplicate);

#endif // EDITOR_H


