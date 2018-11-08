/**
 * Bitmap Import for Imagination Pilots Entertainment 32-bit games (IPE32)
 * - Where's Waldo? Exploring Geography
 * - Eraser Turnabout by Imagination Pilots
 * - Virtual K'Nex by Imagination Pilots
 * ART file packer and unpacker by Daniel Marschall, ViaThinkSoft (C) 2014-2018
 * Revision: 2018-02-15
 **/

#ifndef __inc__ipe32_bmpimport
#define __inc__ipe32_bmpimport

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct tagIpe32BmpImportData {
	size_t dataSize;
	char error[255];
} Ipe32BmpImportData;

bool ipe32_bmp_import(FILE* fibBitmap, Ipe32BmpImportData* result);
void ipe32_free_bmpimport_result(Ipe32BmpImportData *res);

#endif // #ifndef __inc__ipe32_bmpimport

