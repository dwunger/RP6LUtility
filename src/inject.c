#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>

#include "b010_editor.h"

#define MAX_PATH 260

// Structures matching rpack format
struct Header {
    uint32_t sig;
    uint32_t ver;
    uint8_t c1;
    uint8_t c2;
    uint8_t c3;
    uint8_t c4;
    uint32_t parts;
    uint32_t sections;
    uint32_t files;
    uint32_t fnames_size;
    uint32_t fnames;
    uint32_t alignment;
};

struct Section {
    uint8_t filetype;
    uint8_t type2;
    uint8_t type3;
    uint8_t type4;
    uint32_t offset;
    uint32_t unpackedsize;
    uint32_t packedsize;
    uint16_t resourcecount;
    uint16_t unk;
};

struct FilePart {
    uint8_t sectionIndex;
    uint8_t unk1;
    uint16_t fileIndex;
    uint32_t offset;
    uint32_t size;
    uint32_t unk2;
};

struct FileMap {
    uint8_t partsCount;
    uint8_t unk1;
    uint8_t filetype;
    uint8_t unk2;
    uint32_t fileIndex;
    uint32_t firstPart;
};

struct FNameIdx {
    uint32_t offset;
};

// Global structures
static struct Header header;
static struct Section *section = NULL;
static struct FilePart *filepart = NULL;
static struct FileMap *filemap = NULL;
static struct FNameIdx *fname_idx = NULL;

static char modifier[] = "THIS FILE HAS BEEN MODIFIED";

static void OpenFileExist(const char *path) {
    static char editAs[] = "Hex";
    int idx = FindOpenFile(path);
    if (idx != -1) {
        FileSelect(idx);
    } else {
        FileOpen(path, 0, editAs, 0);
    }
}

static const char *GetResourceName(int fileIndex, uint64_t filename_offset) {
    static char filename[MAX_PATH];
    const char *str = ReadString((int)(fname_idx[fileIndex].offset + filename_offset));
    strncpy(filename, str, MAX_PATH - 1);
    filename[MAX_PATH - 1] = '\0';
    return filename;
}

static int GetFileIndex(const char *Filename, uint64_t filename_offset) {
    // Search for filename in the names table
    for (uint32_t k = 0; k < header.fnames; k++) {
        const char *name = ReadString((int)(fname_idx[k].offset + filename_offset));
        if (strcmp(name, Filename) == 0) {
            return (int)k;
        }
    }
    return -1;
}

// DDS format detection - returns rpack format code from DXGI format
static int get_rpack_format_from_dds(uint32_t fourCC, uint32_t flags, uint32_t RGBBitCount,
                                      uint32_t RBitMask, uint32_t GBitMask, uint32_t BBitMask,
                                      uint32_t ABitMask, uint32_t dxgiFormat, int isPassThrough) {
    int format = 0;

    if (flags & 0x00000040) {  // DDS_RGB
        switch (RGBBitCount) {
        case 32:
            if (RBitMask == 0x000000ff && GBitMask == 0x0000ff00 &&
                BBitMask == 0x00ff0000 && ABitMask == 0xff000000) {
                format = isPassThrough ? 32 : 38;
            } else if (RBitMask == 0x0000ffff && GBitMask == 0xffff0000 &&
                       BBitMask == 0x00000000 && ABitMask == 0x00000000) {
                format = 20;
            } else {
                fprintf(stderr, "Unsupported DDS RGB format\n");
                return -1;
            }
            break;
        case 24:
        case 16:
            fprintf(stderr, "Unsupported DDS format (16/24 bit RGB)\n");
            return -1;
        }
    } else if (flags & 0x00020000) {  // DDS_LUMINANCE
        if (RGBBitCount == 8) {
            if (RBitMask == 0x000000ff && GBitMask == 0x00000000 &&
                BBitMask == 0x00000000 && ABitMask == 0x00000000) {
                format = isPassThrough ? 5 : 1;
            }
        }
        if (RGBBitCount == 16) {
            if (RBitMask == 0x0000ffff && GBitMask == 0x00000000 &&
                BBitMask == 0x00000000 && ABitMask == 0x00000000) {
                format = 7;
            }
            if (RBitMask == 0x0000ffff && GBitMask == 0x00000000 &&
                BBitMask == 0x00000000 && ABitMask == 0x0000ff00) {
                format = 15;
            }
        }
    } else if (flags & 0x00000002) {  // DDS_ALPHA
        fprintf(stderr, "Unsupported DDS ALPHA format\n");
        return -1;
    } else if (flags & 0x00000004) {  // DDS_FOURCC
        if (fourCC == 827611204) {        // DXT1
            format = 59;
        } else if (fourCC == 861165636) { // DXT3
            format = 60;
        } else if (fourCC == 894720068) { // DXT5
            format = 61;
        } else if (fourCC == 844388420) { // DXT2
            format = 60;
        } else if (fourCC == 877942852) { // DXT4
            format = 61;
        } else if (fourCC == 826889281) { // ATI1
            format = 63;
        } else if (fourCC == 1429488450) { // BC4U
            format = 63;
        } else if (fourCC == 1395934018) { // BC4S
            format = 62;
        } else if (fourCC == 843666497) { // ATI2
            format = 65;
        } else if (fourCC == 1429553986) { // BC5U
            format = 65;
        } else if (fourCC == 1395999554) { // BC5S
            format = 64;
        } else if (fourCC == 1195525970 || fourCC == 1111970375) { // RGBG/GRGB
            fprintf(stderr, "Unsupported DDS format (RGBG/GRGB)\n");
            return -1;
        } else if (fourCC == 808540228) { // DX10
            switch (dxgiFormat) {
            case 61: format = isPassThrough ? 5 : 1; break;
            case 63: format = 1; break;
            case 56: format = 7; break;
            case 58: format = 8; break;
            case 59: format = 10; break;
            case 49: format = 15; break;
            case 51: format = 16; break;
            case 50: format = 17; break;
            case 35: format = 20; break;
            case 37: format = 21; break;
            case 36: format = 22; break;
            case 28:
            case 29: format = isPassThrough ? 32 : 38; break;
            case 31: format = 39; break;
            case 30: format = 40; break;
            case 10: format = 46; break;
            case 11: format = 47; break;
            case 13: format = 48; break;
            case 71:
            case 72: format = 59; break;
            case 74:
            case 75: format = 60; break;
            case 77:
            case 78: format = 61; break;
            case 81: format = 62; break;
            case 80: format = 63; break;
            case 84: format = 64; break;
            case 83: format = 65; break;
            case 95: format = 66; break;
            case 98: format = 68; break;
            default:
                fprintf(stderr, "Unsupported DX10 format: %u\n", dxgiFormat);
                return -1;
            }
        } else {
            // Legacy fourCC codes
            switch (fourCC) {
            case 36: format = 47; break;  // R16G16B16A16_UNORM
            case 110: format = 48; break; // R16G16B16A16_SNORM
            case 111: format = 6; break;  // R16_FLOAT
            case 112: format = 19; break; // R16G16_FLOAT
            case 113: format = 46; break; // R16G16B16A16_FLOAT
            default:
                fprintf(stderr, "Unsupported DDS fourCC: %u\n", fourCC);
                return -1;
            }
        }
    } else {
        fprintf(stderr, "Unsupported DDS format flags\n");
        return -1;
    }
    return format;
}

int main(int argc, char *argv[]) {
    char rpack[512];
    char repack_folder[512];

    init_emulator();

    // Get rpack file
    if (argc > 1) {
        strncpy(rpack, argv[1], sizeof(rpack) - 1);
        rpack[sizeof(rpack) - 1] = '\0';
        printf("Using rpack file: %s\n", rpack);
    } else {
        const char *selected = InputOpenFileName("Select a rpack to modify",
            "rpack Files (*.rpack)\0*.rpack\0All Files (*.*)\0*.*\0", "rpack");
        if (selected == NULL || strcmp(selected, "") == 0) {
            puts("rpack file not selected.");
            exit(1);
        }
        strncpy(rpack, selected, sizeof(rpack) - 1);
        rpack[sizeof(rpack) - 1] = '\0';
    }

    // Get repack folder
    if (argc > 2) {
        strncpy(repack_folder, argv[2], sizeof(repack_folder) - 1);
        repack_folder[sizeof(repack_folder) - 1] = '\0';
        printf("Using repack folder: %s\n", repack_folder);
    } else {
        const char *folder = InputDirectory("Select a folder of files to inject to rpack", "");
        if (folder == NULL || strcmp(folder, "") == 0) {
            puts("No folder was selected.");
            exit(1);
        }
        strncpy(repack_folder, folder, sizeof(repack_folder) - 1);
        repack_folder[sizeof(repack_folder) - 1] = '\0';
    }

    OpenFileExist(rpack);

    // Read header (36 bytes at offset 0)
    header.sig = ReadUInt(0);
    header.ver = ReadUInt(4);
    header.c1 = ReadUByte(8);
    header.c2 = ReadUByte(9);
    header.c3 = ReadUByte(10);
    header.c4 = ReadUByte(11);
    header.parts = ReadUInt(12);
    header.sections = ReadUInt(16);
    header.files = ReadUInt(20);
    header.fnames_size = ReadUInt(24);
    header.fnames = ReadUInt(28);
    header.alignment = ReadUInt(32);

    // Allocate arrays
    section = malloc(header.sections * sizeof(struct Section));
    filepart = malloc(header.parts * sizeof(struct FilePart));
    filemap = malloc(header.fnames * sizeof(struct FileMap));
    fname_idx = malloc(header.fnames * sizeof(struct FNameIdx));

    if (!section || !filepart || !filemap || !fname_idx) {
        puts("Memory allocation failed");
        exit(1);
    }

    // Read sections (20 bytes each, starting at offset 36)
    uint32_t offset = 36;
    for (uint32_t i = 0; i < header.sections; i++) {
        section[i].filetype = ReadUByte(offset);
        section[i].type2 = ReadUByte(offset + 1);
        section[i].type3 = ReadUByte(offset + 2);
        section[i].type4 = ReadUByte(offset + 3);
        section[i].offset = ReadUInt(offset + 4);
        section[i].unpackedsize = ReadUInt(offset + 8);
        section[i].packedsize = ReadUInt(offset + 12);
        section[i].resourcecount = ReadUShort(offset + 16);
        section[i].unk = ReadUShort(offset + 18);
        offset += 20;
    }

    // Read fileparts (16 bytes each)
    for (uint32_t i = 0; i < header.parts; i++) {
        filepart[i].sectionIndex = ReadUByte(offset);
        filepart[i].unk1 = ReadUByte(offset + 1);
        filepart[i].fileIndex = ReadUShort(offset + 2);
        filepart[i].offset = ReadUInt(offset + 4);
        filepart[i].size = ReadUInt(offset + 8);
        filepart[i].unk2 = ReadUInt(offset + 12);
        offset += 16;
    }

    // Read filemap (12 bytes each)
    for (uint32_t i = 0; i < header.fnames; i++) {
        filemap[i].partsCount = ReadUByte(offset);
        filemap[i].unk1 = ReadUByte(offset + 1);
        filemap[i].filetype = ReadUByte(offset + 2);
        filemap[i].unk2 = ReadUByte(offset + 3);
        filemap[i].fileIndex = ReadUInt(offset + 4);
        filemap[i].firstPart = ReadUInt(offset + 8);
        offset += 12;
    }

    // Read fname_idx (4 bytes each)
    for (uint32_t i = 0; i < header.fnames; i++) {
        fname_idx[i].offset = ReadUInt(offset);
        offset += 4;
    }

    uint64_t filename_offset = 36 + 20 * header.sections + 16 * header.parts + header.files * (12 + 4);
    uint32_t meta_size = 20 * header.sections + 16 * header.parts;

    // Backup/restore logic
    char meta_path[MAX_PATH];
    snprintf(meta_path, MAX_PATH, "%s.meta", rpack);

    uint64_t meta_start = 0;
    unsigned char *meta_backup = malloc(meta_size);
    if (!meta_backup) {
        puts("Memory allocation failed for meta backup");
        exit(1);
    }

    if (FileExists(meta_path)) {
        OpenFileExist(meta_path);
        meta_start = ReadUInt64(0);
        FileClose();

        OpenFileExist(rpack);
        int64_t isFileModified = FindFirst(modifier, 1, 1, 0, 0.0, 1, meta_start, strlen(modifier));

        if (isFileModified > 0) {
            // Restore backup data
            OpenFileExist(meta_path);
            ReadBytes(meta_backup, 8, meta_size);
            OpenFileExist(rpack);
            WriteBytes(meta_backup, 36, meta_size);
            if (FileSize() > meta_start) {
                DeleteBytes((int64_t)meta_start, (int64_t)(FileSize() - meta_start));
            }
            FileSave();
        } else {
            // File may have been updated by developer
            meta_start = FileSize();
            FileSaveRange(meta_path, 36, meta_size);
            OpenFileExist(meta_path);
            InsertBytes(0, 8, 0);
            WriteUInt64(0, meta_start);
            FileSave();
            FileClose();
            OpenFileExist(rpack);
        }
    } else {
        meta_start = FileSize();
        FileSaveRange(meta_path, 36, meta_size);
        OpenFileExist(meta_path);
        InsertBytes(0, 8, 0);
        WriteUInt64(0, meta_start);
        FileSave();
        FileClose();
        OpenFileExist(rpack);
    }

    // Directory structure
    char meta_folder[MAX_PATH];
    char textures_folder[MAX_PATH];
    snprintf(meta_folder, MAX_PATH, "%s\\meta\\", repack_folder);
    snprintf(textures_folder, MAX_PATH, "%s\\textures\\", repack_folder);

    // Find .desc files
    TFileList fl = FindFiles(meta_folder, "*.desc");
    if (fl.filecount < 1) {
        fl = FindFiles(repack_folder, "*.desc");
    }

    if (fl.filecount > 0) {
        // Mark file as modified - InsertBytes first to extend mmap'd file
        size_t file_end = FileSize();
        size_t modifier_len = strlen(modifier);
        InsertBytes((int64_t)file_end, modifier_len + 5, 0);  // Allocate space
        WriteBytes((unsigned char *)modifier, (int64_t)file_end, modifier_len);  // Write modifier
        FileSave();

        // Buffer for file data
        unsigned char *buffer = malloc(120000000);
        if (!buffer) {
            puts("Memory allocation failed for buffer");
            exit(1);
        }

        for (int i = 0; i < fl.filecount; i++) {
            // Extract filename without .desc extension
            char inject_filename[MAX_PATH];
            strncpy(inject_filename, fl.file[i].filename, MAX_PATH - 1);
            inject_filename[MAX_PATH - 1] = '\0';
            size_t len = strlen(inject_filename);
            if (len > 5) {
                inject_filename[len - 5] = '\0';  // Remove .desc
            }

            OpenFileExist(rpack);
            int index = GetFileIndex(inject_filename, filename_offset);

            if (index == -1) {
                printf("Warning: File not found in rpack: %s\n", inject_filename);
                continue;
            }

            printf("Injecting: %s\n", GetResourceName(index, filename_offset));

            // Build paths for old and new directory structures
            char dds_to_inject_old[MAX_PATH], dds_to_inject_new[MAX_PATH], dds_to_inject[MAX_PATH];
            char dds_header_old[MAX_PATH], dds_header_new[MAX_PATH], dds_header[MAX_PATH];

            snprintf(dds_to_inject_old, MAX_PATH, "%s\\%s\\%s.dds", repack_folder, inject_filename, inject_filename);
            snprintf(dds_to_inject_new, MAX_PATH, "%s%s.dds", textures_folder, inject_filename);
            strcpy(dds_to_inject, FileExists(dds_to_inject_old) ? dds_to_inject_old : dds_to_inject_new);

            snprintf(dds_header_old, MAX_PATH, "%s\\%s\\%s.header", repack_folder, inject_filename, inject_filename);
            snprintf(dds_header_new, MAX_PATH, "%s%s.header", meta_folder, inject_filename);
            strcpy(dds_header, FileExists(dds_header_old) ? dds_header_old : dds_header_new);

            for (uint32_t l = 0; l < filemap[index].partsCount; l++) {
                uint32_t partIdx = l + filemap[index].firstPart;
                uint32_t original_padding = 0;
                if ((filepart[partIdx].size) % 16 != 0) {
                    original_padding = 16 - (filepart[partIdx].size) % 16;
                }

                if (filemap[index].filetype != 32) {
                    // Non-texture resource
                    char unknown_path[MAX_PATH];
                    snprintf(unknown_path, MAX_PATH, "%s%d_%s", meta_folder, l, inject_filename);
                    if (!FileExists(unknown_path)) {
                        snprintf(unknown_path, MAX_PATH, "%s\\%s\\%d_%s", repack_folder, inject_filename, l, inject_filename);
                    }

                    OpenFileExist(unknown_path);
                    size_t filesize = FileSize();
                    ReadBytes(buffer, 0, (int)filesize);
                    FileClose();

                    uint32_t padding = 0;
                    if (filesize % 16 != 0) {
                        padding = 16 - (filesize % 16);
                    }

                    OpenFileExist(rpack);

                    // Get new offset and size
                    uint64_t newOffset = (FileSize() >> 4) - section[filepart[partIdx].sectionIndex].offset;
                    uint32_t newSize = (uint32_t)filesize;
                    uint32_t newSectionSize = section[filepart[partIdx].sectionIndex].unpackedsize
                                             - filepart[partIdx].size + (uint32_t)filesize
                                             - original_padding + padding;

                    // Update structures in file
                    uint32_t part_offset = 36 + 20 * header.sections + partIdx * 16;
                    WriteUInt(part_offset + 4, (uint32_t)newOffset);
                    WriteUInt(part_offset + 8, newSize);

                    uint32_t section_offset = 36 + filepart[partIdx].sectionIndex * 20;
                    WriteUInt(section_offset + 8, newSectionSize);

                    // Update in-memory structures
                    filepart[partIdx].offset = (uint32_t)newOffset;
                    filepart[partIdx].size = newSize;
                    section[filepart[partIdx].sectionIndex].unpackedsize = newSectionSize;

                    // Append data
                    size_t write_pos = FileSize();
                    InsertBytes((int64_t)write_pos, (int64_t)(filesize + padding), 0);
                    WriteBytes(buffer, (int64_t)write_pos, (int64_t)filesize);
                } else {
                    // Texture resource
                    if (l == 0) {
                        OpenFileExist(dds_header);
                        size_t filesize = ReadUInt(8);
                        uint32_t headerType = ReadUInt(64);
                        uint8_t tex_type = ReadUByte(71) & 0x03;
                        uint8_t original_format = ReadUByte(70);
                        int isPassThrough = (original_format == 5 || original_format == 32) ? 1 : 0;
                        int format = original_format;

                        if (headerType != 0) {
                            OpenFileExist(dds_to_inject);

                            uint16_t width = (uint16_t)ReadUInt(16);
                            uint16_t height = (uint16_t)ReadUInt(12);
                            uint8_t mip_count = (uint8_t)ReadUInt(28);
                            uint32_t fourCC = ReadUInt(84);
                            uint32_t flags = ReadUInt(80);
                            uint32_t RGBBitCount = ReadUInt(88);
                            uint32_t RBitMask = ReadUInt(92);
                            uint32_t GBitMask = ReadUInt(96);
                            uint32_t BBitMask = ReadUInt(100);
                            uint32_t ABitMask = ReadUInt(104);
                            uint32_t dxgiFormat = (fourCC == 808540228) ? ReadUInt(128) : 0;

                            format = get_rpack_format_from_dds(fourCC, flags, RGBBitCount,
                                RBitMask, GBitMask, BBitMask, ABitMask, dxgiFormat, isPassThrough);
                            if (format < 0) {
                                printf("Error: Unsupported format for %s\n", inject_filename);
                                continue;
                            }

                            FileClose();

                            // Update the header file
                            OpenFileExist(dds_header);
                            WriteUShort(64, width);
                            WriteUShort(66, height);
                            WriteUByte(71, (mip_count << 2) | tex_type);
                            WriteUByte(70, (uint8_t)format);
                            FileSave();
                        }

                        ReadBytes(buffer, 0, (int)filesize);
                        FileClose();

                        uint32_t padding = 0;
                        if (filesize % 16 != 0) {
                            padding = 16 - (filesize % 16);
                        }

                        OpenFileExist(rpack);

                        uint64_t newOffset = (FileSize() >> 4) - section[filepart[partIdx].sectionIndex].offset;
                        uint32_t newSize = (uint32_t)filesize;
                        uint32_t newSectionSize = section[filepart[partIdx].sectionIndex].unpackedsize
                                                 - filepart[partIdx].size + (uint32_t)filesize
                                                 - original_padding + padding;

                        uint32_t part_offset = 36 + 20 * header.sections + partIdx * 16;
                        WriteUInt(part_offset + 4, (uint32_t)newOffset);
                        WriteUInt(part_offset + 8, newSize);

                        uint32_t section_offset_pos = 36 + filepart[partIdx].sectionIndex * 20;
                        WriteUInt(section_offset_pos + 8, newSectionSize);

                        filepart[partIdx].offset = (uint32_t)newOffset;
                        filepart[partIdx].size = newSize;
                        section[filepart[partIdx].sectionIndex].unpackedsize = newSectionSize;

                        size_t write_pos = FileSize();
                        InsertBytes((int64_t)write_pos, (int64_t)(filesize + padding), 0);
                        WriteBytes(buffer, (int64_t)write_pos, (int64_t)filesize);
                    } else {
                        // Part > 0: inject DDS data
                        OpenFileExist(dds_to_inject);
                        size_t dds_filesize = FileSize();

                        // Determine texture offset (skip DDS header)
                        uint32_t tex_offset = (ReadUInt(84) == 808540228) ? 148 : 128;
                        size_t filesize = dds_filesize - tex_offset;

                        ReadBytes(buffer, (int)tex_offset, (int)filesize);
                        FileClose();

                        uint32_t padding = 0;
                        if (filesize % 16 != 0) {
                            padding = 16 - (filesize % 16);
                        }

                        OpenFileExist(rpack);

                        uint64_t newOffset = (FileSize() >> 4) - section[filepart[partIdx].sectionIndex].offset;
                        uint32_t newSize = (uint32_t)filesize;
                        uint32_t newSectionSize = section[filepart[partIdx].sectionIndex].unpackedsize
                                                 - filepart[partIdx].size + (uint32_t)filesize
                                                 - original_padding + padding;

                        uint32_t part_offset = 36 + 20 * header.sections + partIdx * 16;
                        WriteUInt(part_offset + 4, (uint32_t)newOffset);
                        WriteUInt(part_offset + 8, newSize);

                        uint32_t section_offset_pos = 36 + filepart[partIdx].sectionIndex * 20;
                        WriteUInt(section_offset_pos + 8, newSectionSize);

                        filepart[partIdx].offset = (uint32_t)newOffset;
                        filepart[partIdx].size = newSize;
                        section[filepart[partIdx].sectionIndex].unpackedsize = newSectionSize;

                        size_t write_pos = FileSize();
                        InsertBytes((int64_t)write_pos, (int64_t)(filesize + padding), 0);
                        WriteBytes(buffer, (int64_t)write_pos, (int64_t)filesize);
                    }
                }
            }
        }

        free(buffer);
        OpenFileExist(rpack);
        FileSave();
        printf("Injection Completed.\n");
    } else {
        printf("No .desc files found in %s\n", repack_folder);
    }

    FreeFindFiles(&fl);
    free(meta_backup);
    free(section);
    free(filepart);
    free(filemap);
    free(fname_idx);

    return 0;
}
