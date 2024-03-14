#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>

#include "b010_editor.h"
#define MAX_PATH 260 // May include windows.h if I/O overhead is an issue

//Filter textures not matching pattern:
char texture_pattern[] = "sky";

/*Comment out any line to enable extraction*/
/*Example: will extract all excluding diffuse textures */

dif_mask = "_dif"; //diffuse map
//nrm_mask = "_nrm"; // Normal map
//rgh_mask = "_rgh"; // Roughness map
//clp_mask = "_clp"; // Clip map
//msk_mask = "_msk"; // Mask map
//spc_mask = "_spc"; // Specular map
//trn_mask = "_trn"; // Transparency map
//ocl_mask = "_ocl"; // Occlusion map
//opc_mask = "_opc"; // Opacity map
//dpt_mask = "_dpt"; // Depth map ??
//det_mask = "_det"; // Detail map ??
//idx_mask = "_idx"; // Index map ??


//dds header writer
int isDXT10 = 1;

//base headerSize is 124 and automatically expands

int bpp = 0;
int isCompressed = 0;

unsigned int pitch = 0;

unsigned int DDSD_CAPS           = 0x00000001;
unsigned int DDSD_HEIGHT         = 0x00000002;
unsigned int DDSD_WIDTH          = 0x00000004;
unsigned int DDSD_PITCH          = 0x00000008;
unsigned int DDSD_PIXELFORMAT    = 0x00001000;
unsigned int DDSD_MIPMAPCOUNT    = 0x00020000;
unsigned int DDSD_LINEARSIZE     = 0x00080000;
unsigned int DDSD_DEPTH          = 0x00800000;

unsigned int DDSCAPS_COMPLEX     = 0x00000008;
unsigned int DDSCAPS_TEXTURE     = 0x00001000;
unsigned int DDSCAPS_MIPMAP      = 0x00400000;
unsigned int DDSCAPS2_VOLUME     = 0x00200000;
unsigned int DDSCAPS2_CUBEMAP    = 0x00000200;
unsigned int DDSCAPS2_CUBEMAP_ALL_FACES = 0x0000FC00;

unsigned int DDPF_ALPHAPIXELS    = 0x00000001;
unsigned int DDPF_ALPHA          = 0x00000002;
unsigned int DDPF_FOURCC         = 0x00000004;
unsigned int DDPF_RGB            = 0x00000040;
unsigned int DDPF_NORMAL         = 0x80000000;

char haystack[];
char needle[];

int contains( char haystack[], char needle[] );
int isExcluded(const char *file);
unsigned int computePitch(unsigned int width, unsigned int height, int bpp, int isCompressed); 
int get_dxt_header(int format); 
int get_bpp(int format);
//required flags for dds
unsigned int flags = 0, hasFOURCC = 0;
void dds_Generate(unsigned int width, unsigned int height, unsigned int mip_count, unsigned int format, unsigned int tex_type, unsigned int depth); 
void OpenFileExist(char path[]);
string GetResourceName(int fileIndex); 
string GetResourceSavePath(string ResourceName, int Part, int IsTexture); 

int main(int argc, char *argv[]) 
{
	char rpack[512];
	strcpy(rpack, InputOpenFileName("Select a file to unpack", "rpack (*.rpack)", ""));

	if ((strcmp(rpack, "") != 0)) {
		puts("rpack file not selected.");
		exit(1);
	}
	OpenFileExist(rpack);

	RunTemplate("rp6l.bt");

	int i,j;
	string s,savepath;

	//Set buffer for textures ~20MB
	unsigned char buffer[20000000+80];

	char rpack_name[512];

	FileNameGetBase(rpack);
	char rpack_basename[] = SubStr(rpack_name, 0, Strlen(rpack_name) - 6);
	char rpack_path[] = FileNameGetPath(rpack);

	uint64_t file_offset, file_size, filename_offset;

	filename_offset = 36 + 20 * header.sections + 16 * header.parts + header.files * (12 + 4);


	int IsTexture = 0;
	int headerSize, headerType, width, height, format, mip_count, depth, tex_type;
	//header.files corresponds to total number of files in rpack
	//
	for (i = 0; i < header.files; i++) 
	{
		for (j = 0; j < filemap[i].partsCount; j++) 
		{
			OpenFileExist(rpack);
			// Test GetResourceName for pattern defined in header
			if (Strstr(GetResourceName(i), texture_pattern) == -1 || isExcluded(GetResourceName(i))) 
			{
				continue;  // If not found, skip to next iteration of the loop.
			}

			//File Size
			file_size = filepart[filemap[i].firstPart + j].size;
			//File Offset
			file_offset = filepart[filemap[i].firstPart + j].offset;
			file_offset = file_offset << 4;
			file_offset = file_offset + (section[filepart[filemap[i].firstPart + j].sectionIndex].offset << 4);
			//Detect if compressed
			if ((section[filepart[filemap[i].firstPart + j].sectionIndex].packedsize == 0)) {
				puts("Can not extract compressed files, extraction aborted");
				exit(1);
			}
			//Extract
			IsTexture = (filemap[i].filetype == 32) ? 1 : 0;
			savepath = GetResourceSavePath(GetResourceName(i), j, IsTexture);

			if (IsTexture == 1) 
			{
				if (j == 0) 
				{
					savepath = GetResourceSavePath(GetResourceName(i), j, IsTexture);
					FileSaveRange(savepath + ".header", file_offset, file_size);
					headerSize = ReadUInt(file_offset + 8);
					headerType = ReadUInt(file_offset + 64);
					width = ReadUShort(file_offset + 64);
					height = ReadUShort(file_offset + 66);
					format = ReadUByte(file_offset + 70);
					depth = ReadUByte(file_offset + 68);
					mip_count = (ReadUByte(file_offset + 71) >> 2);
					tex_type = (ReadUByte(file_offset + 71) & 0x03);    // 0 = 2d, 1 = cubemap, 2 = 3d
				} 
				else 
				{
					if (headerType != 0) 
					{
						savepath = GetResourceSavePath(GetResourceName(i), j, IsTexture);
						FileSaveRange(savepath + ".dds", file_offset, file_size);
						OpenFileExist(savepath + ".dds");
						dds_Generate( width,  height,  mip_count,  format,  tex_type,  depth);
						FileSave();
						FileClose();
					}
				}
			} 
			else 
			{
				FileSaveRange(savepath, file_offset, file_size);
			}
			FileSelect(FindOpenFileW(rpack_path + rpack_name));
		}
	}

	//header.bin
	FileSaveRange(rpack_path + rpack_basename + "_unpack\\meta\\header.bin", 0, 36);

	//section.bin
	buffer[0] = header.sections;

	for (i = 0; i < header.sections; i++) 
	{
		buffer[5*i + 1] = section[i].filetype;
		buffer[5*i + 2] = section[i].type2;
		buffer[5*i + 3] = section[i].type3;
		buffer[5*i + 4] = section[i].type4;
		buffer[5*i + 5] = section[i].unk;
	}
	unsigned int buffersize = header.sections * 5 + 1;
	FileSaveRange(rpack_path + rpack_basename + "_unpack\\meta\\section.bin", 0, 0);
	FileOpen(rpack_path + rpack_basename + "_unpack\\meta\\section.bin", false, "Hex", false);
	WriteBytes(buffer, 0, buffersize);
	FileSave();
	FileClose();
	FileSelect(FindOpenFileW(rpack_path + rpack_name));

	//file desc
	for (i = 0; i < header.files; i++) 
	{
		// Test for pattern defined in header
		if (Strstr(GetResourceName(i), texture_pattern) == -1 || isExcluded(GetResourceName(i)))
		{
			continue;  // If not found, skip to next iteration of the loop.
		}
		buffer[0] = filemap[i].partsCount;
		buffer[1] = filemap[i].filetype;
		buffer[2] = filemap[i].unk2;
		for (j = 0; j < filemap[i].partsCount; j++) 
		{
			buffer[3 + j * 2] = filepart[filemap[i].firstPart + j].sectionIndex;
			buffer[4 + j * 2] = filepart[filemap[i].firstPart + j].unk1;
		}
		buffersize = 3 + filemap[i].partsCount * 2;
		savepath = rpack_path + rpack_basename + "_unpack\\meta\\" + GetResourceName(i) + ".desc";
		FileSaveRange(savepath, 0, 0);
		FileOpen(savepath, false, "Hex", false);
		WriteBytes(buffer, 0, buffersize);
		FileSave();
		FileClose();
		FileSelect(FindOpenFileW(rpack_path + rpack_name));
	}

	return 0;
}

int contains( char haystack[], char needle[] ) {
    if (Strstr(haystack, needle) != -1)
    {
        return 1;
    }
    return 0;
}

int isExcluded(const char* file) {
    if (dif_mask != NULL && strstr(file, dif_mask) != NULL) {
		return 1;
    }
    if (nrm_mask != NULL && strstr(file, nrm_mask) != NULL) {
		return 1;
    }
    if (rgh_mask != NULL && strstr(file, rgh_mask) != NULL) {
		return 1;
    }
    if (clp_mask != NULL && strstr(file, clp_mask) != NULL) {
		return 1;
    }
    if (msk_mask != NULL && strstr(file, msk_mask) != NULL) {
		return 1;	
    }
    if (spc_mask != NULL && strstr(file, spc_mask) != NULL) {
		return 1;
    }
    if (trn_mask != NULL && strstr(file, trn_mask) != NULL) {
		return 1;
    }
	if (ocl_mask != NULL && strstr(file, ocl_mask) != NULL) {
		return 1;
    }
    if (opc_mask != NULL && strstr(file, opc_mask) != NULL) {
		return 1;
    }
    if (dpt_mask != NULL && strstr(file, dpt_mask) != NULL) {
		return 1;
    }
    if (det_mask != NULL && strstr(file, det_mask) != NULL) {
		return 1;
    }
    if (idx_mask != NULL && strstr(file, idx_mask) != NULL) {
		return 1;
    }
    return 0;
}

unsigned int computePitch(unsigned int width, unsigned int height, int bpp, int isCompressed) 
{
	if (isCompressed == 1) 
	{
		unsigned int pitch = ((width + 3) / 4) * ((height + 3) / 4) * bpp * 2;	
	} 
	else 
	{
		unsigned int pitch = ( width * bpp + 7 ) / 8;
	}
	return pitch;
}

unsigned int computePitch(unsigned int width, unsigned int height, int bpp, int isCompressed) 
{
	if (isCompressed == 1) 
	{
		unsigned int pitch = ((width + 3) / 4) * ((height + 3) / 4) * bpp * 2;	
	} 
	else 
	{
		unsigned int pitch = ( width * bpp + 7 ) / 8;
	}
	return pitch;
}

int get_dxt_header(int format) 
{
	switch (format) 
	{
		case 0:     //R8_UNORM
			format = 61;
			break;
		case 1:     //R8_SNORM
			format = 63;
			break;
		case 5:     //R8_UNORM 
			format = 61;
			break;
		case 7:     //R16_UNORM
			format = 56;
			break;
		case 8:     //R16_SNORM
			format = 58;
			break;
		case 10:    //R16_SINT
			format = 59;
			break;
		case 15:    //R8G8_UNORM
			format = 49;
			break;
		case 16:    //R8G8_SNORM
			format = 51;
			break;
		case 17:    //R8G8_UINT
			format = 50;
			break;
		case 20:    //R16G16_UNORM
			format = 35;
			break;
		case 21:    //R16G16_SNORM
			format = 37;
			break;
		case 22:    //R16G16_UINT
			format = 36;
			break;
		case 32:    //R8G8B8A8_UNORM 
			format = 28;
			break;
		case 38:    //R8G8B8A8_UNORM
			format = 28;
			break;
		case 39:    //R8G8B8A8_SNORM
			format = 31;
			break;
		case 40:    //R8G8B8A8_UINT
			format = 30;
			break;
		case 46:    //R16G16B16A16_FLOAT
			format = 10;
			break;
		case 47:    //R16G16B16A16_UNORM
			format = 11;
			break;
		case 48:    //R16G16B16A16_SNORM
			format = 13;
			break;
		case 59:    //BC1_UNORM
			format = 71;
			break;
		case 62:    //BC4_SNORM
			format = 81;
			break;
		case 63:    //BC4_UNORM
			format = 80;
			break;
		case 64:    //BC5_SNORM
			format = 84;
			break;
		case 65:    //BC5_UNORM
			format = 83;
			break;
		case 66:    //BC6H_UF16
			format = 95;
			break;
		case 68:    //BC7_UNORM
			format = 98;
			break;
		default:
			puts("Unsupported DDS format detected.");
			exit(1);	
			break;
	}	
    return format;
}

int get_dxt_header(int format) 
{
	switch (format) 
	{
		case 0:     //R8_UNORM
			format = 61;
			break;
		case 1:     //R8_SNORM
			format = 63;
			break;
		case 5:     //R8_UNORM 
			format = 61;
			break;
		case 7:     //R16_UNORM
			format = 56;
			break;
		case 8:     //R16_SNORM
			format = 58;
			break;
		case 10:    //R16_SINT
			format = 59;
			break;
		case 15:    //R8G8_UNORM
			format = 49;
			break;
		case 16:    //R8G8_SNORM
			format = 51;
			break;
		case 17:    //R8G8_UINT
			format = 50;
			break;
		case 20:    //R16G16_UNORM
			format = 35;
			break;
		case 21:    //R16G16_SNORM
			format = 37;
			break;
		case 22:    //R16G16_UINT
			format = 36;
			break;
		case 32:    //R8G8B8A8_UNORM 
			format = 28;
			break;
		case 38:    //R8G8B8A8_UNORM
			format = 28;
			break;
		case 39:    //R8G8B8A8_SNORM
			format = 31;
			break;
		case 40:    //R8G8B8A8_UINT
			format = 30;
			break;
		case 46:    //R16G16B16A16_FLOAT
			format = 10;
			break;
		case 47:    //R16G16B16A16_UNORM
			format = 11;
			break;
		case 48:    //R16G16B16A16_SNORM
			format = 13;
			break;
		case 59:    //BC1_UNORM
			format = 71;
			break;
		case 62:    //BC4_SNORM
			format = 81;
			break;
		case 63:    //BC4_UNORM
			format = 80;
			break;
		case 64:    //BC5_SNORM
			format = 84;
			break;
		case 65:    //BC5_UNORM
			format = 83;
			break;
		case 66:    //BC6H_UF16
			format = 95;
			break;
		case 68:    //BC7_UNORM
			format = 98;
			break;
		default:
			puts("Unsupported DDS format detected.");
			exit(1);	
			break;
	}	
    return format;
}

int get_bpp(int format)
{
	switch(format) 
	{
		case 0:
		case 1:
		case 5:
			bpp = 8;	//R8_UNORM, R8_SNORM, R8_UNORM
			flags = flags | DDSD_PITCH;
			break;
		case 7:
		case 8:
		case 10:
		case 15:
		case 16:
		case 17:
			bpp = 16;	//R16_UNORM, R16_SNORM, R16_SINT, R8G8_UNORM, R8G8_SNORM
			flags = flags | DDSD_PITCH;
			break;
		case 20:
		case 21:
		case 22:
		case 32:
		case 38:
		case 39:
		case 40:
			bpp = 32;	//R16G16_UNORM, R16G16_SNORM, R16G16_UINT, R8G8B8A8_UNORM, R8G8B8A8_SNORM, R8G8B8A8_UINT
			flags = flags | DDSD_PITCH;
			break;
		case 46:
		case 47:
		case 48:
			bpp = 64;	//R16G16B16A16_FLOAT, R16G16B16A16_UNORM, R16G16B16A16_SNORM
			flags = flags | DDSD_PITCH;
			break;
		case 59:
		case 62:
		case 63:
			bpp = 4;	//BC1_UNORM, BC4_SNORM, BC4_UNORM
			isDXT10 = 0;
			isCompressed = 1;
			flags = flags | DDSD_LINEARSIZE;
			hasFOURCC = 1;
			break;
		case 64:
		case 65:
		case 66:
		case 68:
			bpp = 8;	//BC5_SNORM, BC5_UNORM, BC6H_UF16, BC7_UNORM
			isCompressed = 1;
			flags = flags | DDSD_LINEARSIZE;
			hasFOURCC = 2;
			break;
		default:
			puts("Unsupported texture format detected.");
			exit(1);		
			break;
	}
    return format;
}

void dds_Generate(unsigned int width, unsigned int height, unsigned int mip_count, unsigned int format, unsigned int tex_type, unsigned int depth) 
{
    InsertBytes(0, 4 + 124, 0);
	isDXT10 = 0;
	isCompressed = 0;
	hasFOURCC = 0;
	flags = DDSD_CAPS | DDSD_PIXELFORMAT | DDSD_WIDTH | DDSD_HEIGHT | DDSD_MIPMAPCOUNT;
	//bits per pixels
    format = get_bpp(format);
    
	if (depth > 1) 
	{
		flags = flags | DDSD_DEPTH;
	}
	
    WriteUInt(0, 542327876);                                       //DDS 
    WriteUInt(4, 124);                                             //generic headerSize
    
    WriteUInt(8, flags);                                           //flags
	WriteUInt(12, height);	                                       //height
	WriteUInt(16, width);	                                       //width
	WriteUInt(20, computePitch(width, height, bpp, isCompressed)); //pitch
	WriteUInt(24, depth);                                          //depth
	WriteUInt(28, mip_count);	                                   //mip_count
	
	//ddspf
	WriteUInt(76, 32); //size
	
	flags = DDPF_FOURCC;
	//use DX10 for uncompressed and except dxt1
	WriteUInt(80, flags); 	//flags

	if (format == 59) 
	{
		WriteUInt(84, 827611204);
	} 
	else 
	{
		WriteUInt(84, 808540228);
		InsertBytes(128, 20, 0);
	}
	
	//caps
	flags = DDSCAPS_TEXTURE;
	if (mip_count > 1) 
	{
		flags = flags | DDSCAPS_COMPLEX;
	}
	
	if (depth > 1) 
	{
		flags = flags | DDSCAPS_COMPLEX;
	}
	
	if (tex_type != 0) 
	{
		flags = flags | DDSCAPS_COMPLEX;
	}
	WriteUInt(108, flags);
	
	//caps2
    flags = 0;
	if (tex_type == 1) 
	{
		flags = DDSCAPS2_CUBEMAP;
	} 
	else if (tex_type == 2) 
	{
		flags = DDSCAPS2_VOLUME;
	}
	WriteUInt(112, flags);
	
    if (format != 59) 
	{
	    //dxt10 header
		format = get_dxt_header(format);

	    WriteUInt(128, format);	//dxgiFormat
	    if (tex_type == 2) 
		{
		    WriteUInt(132, 4);
	    } 
		else 
		{
		    WriteUInt(132, 3);
	    }
	    if (tex_type == 1) 
		{
		    WriteUInt(136, 4);	//miscFlag
	    }
	    WriteUInt(140, 1);	//arraySize
    }
	
}

void OpenFileExist(char path[]) 
{
    if (FindOpenFile(path) != -1) 
	{
        FileSelect(FindOpenFile(path));
    } 
	else 
	{
        FileOpen(path, false, "Hex", false);
    }
}

string GetResourceSavePath(string ResourceName, int Part, int IsTexture) 
{
	string savepath = rpack_path + rpack_basename + "_unpack\\textures\\";
	if (Part == 0)
	{
		savepath = rpack_path + rpack_basename + "_unpack\\meta\\";
	}

	MakeDir(savepath);
	if (IsTexture == 1) 
	{
		savepath += ResourceName;
	} 
	else 
	{
		savepath += SPrintf( s, "%d", Part) + "_" + ResourceName;
	}
	return savepath;
}

string GetResourceName(int fileIndex) 
{
	string filename = ReadString(fname_idx[fileIndex].offset + filename_offset);
	return filename;
}

