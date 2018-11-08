/**
 * Bitmap Import for Imagination Pilots Entertainment 16-bit games (IPE16)
 * - Blown Away - The Interactive Game by Imagination Pilots (BA)
 * - Panic in the Park - The Interactive Game by Imagination Pilots (PiP)
 * - Where's Waldo? At the Circus (Waldo1)
 * ART file packer and unpacker by Daniel Marschall, ViaThinkSoft (C) 2014-2018
 * Revision: 2018-02-15
 **/

#ifndef __inc__ipe16_bmpimport
#define __inc__ipe16_bmpimport

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "bitmap.h"
#include "ipe16_artfile.h"

typedef struct tagIpe16BmpImportData {
	Ipe16ColorTable* colorTable;
	unsigned char* bmpData;
	size_t bmpDataSize;
	unsigned int width;
	unsigned int height;
	char error[255];
} Ipe16BmpImportData;

bool ipe16_bmp_import(FILE* fibBitmap, Ipe16BmpImportData* result);
void ipe16_free_bmpimport_result(Ipe16BmpImportData *res);

#endif // #ifndef __inc__ipe16_bmpimport

