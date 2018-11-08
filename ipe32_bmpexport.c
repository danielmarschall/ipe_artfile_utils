/**
 * Bitmap Export for Imagination Pilots Entertainment 32-bit games (IPE32)
 * - Where's Waldo? Exploring Geography
 * - Eraser Turnabout by Imagination Pilots
 * - Virtual K'Nex by Imagination Pilots
 * ART file packer and unpacker by Daniel Marschall, ViaThinkSoft (C) 2014-2018
 * Revision: 2018-02-15
 **/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "bitmap.h"
#include "ipe32_bmpexport.h"

void ipe32_write_bmp(FILE* output, unsigned char* imagedata, size_t imagedata_len) {
	BITMAPFILEHEADER bh={0};
	bh.bfType = BI_SIGNATURE;
	bh.bfSize = sizeof(bh) + imagedata_len;
	bh.bfReserved1 = 0;
	bh.bfReserved2 = 0;
	bh.bfOffBits = 0x436;

	fwrite(&bh, 1, sizeof(bh), output);
	fwrite(imagedata, 1, imagedata_len, output);
}

