/**
 * ART files for Imagination Pilots Entertainment 32-bit games (IPE32)
 * - Where's Waldo? Exploring Geography
 * - Eraser Turnabout by Imagination Pilots
 * - Virtual K'Nex by Imagination Pilots
 * ART file packer and unpacker by Daniel Marschall, ViaThinkSoft (C) 2018
 * Revision: 2018-02-15
 **/

#ifndef __inc__ipe32_artfile
#define __inc__ipe32_artfile

#include <stdint.h>

#define IPE32_COMPRESSIONTYPE_LZW 0
#define IPE32_COMPRESSIONTYPE_NONE 1

#define IPE32_NAME_SIZE 8

#define IPE32_MAGIC_ART "ART_DATA"

#pragma pack(push, 1)

typedef struct tagIpe32FileHeader {
	char       magic[IPE32_NAME_SIZE];   // always "ART_DATA"
	uint32_t   totalHeaderSize;          // size of all headers (file header and all picture headers). headerSize/16 = numberPictures
	uint32_t   reserved;                 // always 0
} Ipe32FileHeader;

typedef struct tagIpe32PictureEntryHeader {
	char       name[IPE32_NAME_SIZE];
	uint32_t   offset;                  // offset to the picture (Ipe32PictureHeader)
	uint32_t   uncompressedSize;        // size of the picture (picture data + palette)
} Ipe32PictureEntryHeader;

/*
typedef struct tagIpe32PictureChunk {
	unsigned compressionType : 1;   // Compression type of the follow-up data
	                                // 0 = LZW-like compression (special implementation)
	                                // 1 = None
	unsigned chunkDataSize : 15;    // size of the chunk data
	//char         data[];
} Ipe32PictureChunk;
*/

#pragma pack(pop)

#endif // #ifndef __inc__ipe32_artfile

