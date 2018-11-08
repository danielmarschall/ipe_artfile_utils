/**
 * Bitmap Export for Imagination Pilots Entertainment 32-bit games (IPE32)
 * - Where's Waldo? Exploring Geography
 * - Eraser Turnabout by Imagination Pilots
 * - Virtual K'Nex by Imagination Pilots
 * ART file packer and unpacker by Daniel Marschall, ViaThinkSoft (C) 2014-2018
 * Revision: 2018-02-15
 **/

#ifndef __inc__ipe32_bmpexport
#define __inc__ipe32_bmpexport

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

void ipe32_write_bmp(FILE* output, unsigned char* imagedata, size_t imagedata_len);

#endif // #ifndef __inc__ipe32_bmpexport

