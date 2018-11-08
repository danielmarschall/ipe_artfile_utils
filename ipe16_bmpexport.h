/**
 * Bitmap Export for Imagination Pilots Entertainment 16-bit games (IPE16)
 * - Blown Away - The Interactive Game by Imagination Pilots (BA)
 * - Panic in the Park - The Interactive Game by Imagination Pilots (PiP)
 * - Where's Waldo? At the Circus (Waldo1)
 * ART file packer and unpacker by Daniel Marschall, ViaThinkSoft (C) 2014-2018
 * Revision: 2018-02-15
 **/

#ifndef __inc__ipe16_bmpexport
#define __inc__ipe16_bmpexport

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "bitmap.h"
#include "ipe16_artfile.h"

void ipe16_write_bmp(FILE* output, unsigned int width, unsigned int height, unsigned char* imagedata, size_t imagedata_len, Ipe16ColorTable ct);

#endif // #ifndef __inc__ipe16_bmpexport

