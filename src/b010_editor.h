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

// 010 editor's variants of GLIBC functions from String.h & stdio variants
int Strcmp(const char *str1, const char *str2);

const char *Strstr(const char *str, const char *substr);

const char *SubStr(const char *str, int start, int len);

int Strlen(const char *str);

const char *SPrintf(const char *format, ...);

#endif // EDITOR_H