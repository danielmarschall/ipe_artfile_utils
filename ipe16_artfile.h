/**
 * ART files for Imagination Pilots Entertainment 16-bit games (IPE16)
 * - Blown Away - The Interactive Game by Imagination Pilots (BA)
 * - Panic in the Park - The Interactive Game by Imagination Pilots (PiP)
 * - Where's Waldo? At the Circus (Waldo1)
 * ART file packer and unpacker by Daniel Marschall, ViaThinkSoft (C) 2014-2018
 * Revision: 2018-02-15
 **/

#ifndef __inc__ipe16_artfile
#define __inc__ipe16_artfile

#include <stdint.h>

#define BA_COMPRESSIONTYPE_LZW 'P'
#define BA_COMPRESSIONTYPE_NONE 'p'

// PANIC.EXE offset 0x44365: Choice between "Q" and "q"
#define PIP_COMPRESSIONTYPE_LZW 'Q'
#define PIP_COMPRESSIONTYPE_NONE 'q'

#define IPE16_PALETTETYPE_ATTACHED 'X'

// Pictures with the type 'C' do not have a palette.
// They use the palette of their parent picture where they are embedded in
#define IPE16_PALETTETYPE_PARENT 'C'

#define IPE16_NAME_SIZE 23

#define IPE16_MAGIC_ART "Art"
#define IPE16_MAGIC_DUMMY '?'

#pragma pack(push, 1)

typedef struct tagIpe16FileHeader {
	char       magic[IPE16_NAME_SIZE]; // always "Art\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" (ignored by the game)
	char       dummy;                  // always '?' (ignored by the game)
	uint32_t   numHeaderEntries;       // number of headers including this file header (number of follow-up Ipe16PictureEntryHeader entries plus one)
	uint32_t   totalFileSize;          // total file size including this header (ignored by the game)
} Ipe16FileHeader;

typedef struct tagIpe16PictureEntryHeader {
	char       name[IPE16_NAME_SIZE];  // zero terminated string. case sensitive
	char       paletteType;            // 'X' (0x58) = RGB palette attached
	                                   // 'C' (0x43) = no palette attached (for embedded picture, use palette of parent picture)
	uint32_t   offset;                 // offset to the picture (PictureHeader)
	uint32_t   size;                   // size of the picture (PictureHeader + picture data + optional palette)
} Ipe16PictureEntryHeader;

typedef struct tagBAPictureHeader {
	char     compressionType;       // Compression type of the follow-up data (top down pixel data; the palette won't be compressed)
	                                // 'P' (0x50, upper case) = LZW compression, more precisely:
	                                //                          The LZW variant of the GIF specification,
	                                //                          but without splitting the output data into chunks
	                                // 'p' (0x70, lower case) = No compression
	uint16_t width;                 // width of the picture
	uint16_t height;                // height of the picture
} BAPictureHeader;

typedef struct tagPipPictureHeader {
	char     compressionType;       // Compression type of the follow-up data (top down pixel data; the palette won't be compressed)
	                                // 'Q' (0x51, upper case) = LZW compression, more precisely:
	                                //                          The LZW variant of the GIF specification,
	                                //                          but without splitting the output data into chunks
	                                // 'q' (0x71, lower case) = No compression
	uint16_t offsetX;               // Additional offsets for 'fine-tuning'
	uint16_t offsetY;
	uint16_t width;                 // width of the picture
	uint16_t height;                // height of the picture
} PipPictureHeader;

typedef struct tagIpe16ColorTableEntry {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} Ipe16ColorTableEntry;

typedef struct tagIpe16ColorTable {
	Ipe16ColorTableEntry colors[256];
} Ipe16ColorTable;

#pragma pack(pop)

#endif // #ifndef __inc__ipe16_artfile

