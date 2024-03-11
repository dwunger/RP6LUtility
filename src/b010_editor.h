#ifndef EDITOR_H
#define EDITOR_H

void init_emulator();

void InsertBytes(int offset, int size, int value);

int FindOpenFile(const char *path);

void FileSelect(int index);

int FileOpen(const char *filename, int runTemplate, const char *editAs, int openDuplicate);

const char *FileNameGetBase(const char *path);

const char *FileNameGetPath(const char *path);

const char *ReadString(int offset);

void MakeDir(const char *path);

void FileSaveRange(const char *path, int offset, int size);

int FindOpenFileW(const char *path);

void WriteBytes(unsigned char *buffer, int offset, int size);

const char *InputOpenFileName(const char *title, const char *filter, const char *default_ext);

void RunTemplate(const char *template_name);

#endif // EDITOR_H