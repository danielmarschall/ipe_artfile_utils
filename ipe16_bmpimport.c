/**
 * Bitmap Import for Imagination Pilots Entertainment 16-bit games (IPE16)
 * - Blown Away - The Interactive Game by Imagination Pilots (BA)
 * - Panic in the Park - The Interactive Game by Imagination Pilots (PiP)
 * - Where's Waldo? At the Circus (Waldo1)
 * ART file packer and unpacker by Daniel Marschall, ViaThinkSoft (C) 2014-2018
 * Revision: 2018-02-15
 **/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "bitmap.h"
#include "ipe16_bmpimport.h"

Ipe16ColorTableEntry rgbquad_to_ipe16_rgb(RGBQUAD rq) {
	Ipe16ColorTableEntry cte;
	cte.r = rq.rgbRed;
	cte.g = rq.rgbGreen;
	cte.b = rq.rgbBlue;
	return cte;
}

bool ipe16_bmp_import(FILE* fibBitmap, Ipe16BmpImportData* result) {
	BITMAPFILEHEADER bitmapFileHeader;
	BITMAPINFOHEADER bitmapInfoHeader;

	#define EXIT_ERROR(msg) { sprintf(result->error, msg); return false; }

	fseek(fibBitmap, 0, SEEK_SET);

	if (!fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, fibBitmap) ||
	    (bitmapFileHeader.bfType != BI_SIGNATURE) ||
	    !fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, fibBitmap)) {
		EXIT_ERROR("Not a bitmap file");
	}

	if (bitmapInfoHeader.biCompression != BI_RGB) {
		EXIT_ERROR("At the moment, only uncompressed files can be read.");
	}

	if (bitmapInfoHeader.biBitCount != 8) {
		EXIT_ERROR("The color depth has to be 8 bpp.");
	}

	Ipe16ColorTable* ct = malloc(sizeof(Ipe16ColorTable));
	#define NUM_COLORS 256
	RGBQUAD rgba_colortable[NUM_COLORS];
	if (!fread(&rgba_colortable, sizeof(rgba_colortable), 1, fibBitmap)) {
		EXIT_ERROR("Error reading color table.");
	}
	int i;
	for (i=0; i<NUM_COLORS; ++i) {
		Ipe16ColorTableEntry x;
		x = rgbquad_to_ipe16_rgb(rgba_colortable[i]);
		ct->colors[i] = x;
	}

	fseek(fibBitmap, bitmapFileHeader.bfOffBits, SEEK_SET);

	const uint32_t realwidth = bitmapInfoHeader.biWidth;
	const uint32_t realheight = abs(bitmapInfoHeader.biHeight);
	const size_t bmpDataSize = realwidth*realheight;
	unsigned char* bmpData = (unsigned char*)malloc(bmpDataSize);
	assert(bmpData != NULL);
	const int padded_width = (bitmapInfoHeader.biWidth + 3) & ~0x03; // http://stackoverflow.com/a/2022194/3544341
	unsigned char* bmpLine = (unsigned char*)malloc(padded_width);
	assert(bmpLine != NULL);
	int h;
	for (h=0; h<realheight; ++h) {
		if (fread(bmpLine, padded_width, 1, fibBitmap) != 1) {
			free(bmpLine);
			free(bmpData);
			free(ct);
			EXIT_ERROR("Error while reading pixel data.");
		}
		int idx;
		if (bitmapInfoHeader.biHeight > 0) {
			// "bottom-up" (most usual)
			// Convert to "top-down":
			idx = (realheight-1)-h;
		} else {
			// "top-down"
			idx = h;
		}
		assert(idx*realwidth >= 0);
		memcpy(bmpData+idx*realwidth, bmpLine, realwidth);
	}
	free(bmpLine);

	result->colorTable = ct;
	result->bmpData = bmpData;
	result->bmpDataSize = bmpDataSize;
	result->width = realwidth;
	result->height = realheight;
	result->error[0] = 0;
	return true;
}

void ipe16_free_bmpimport_result(Ipe16BmpImportData *res) {
	if (res->colorTable) free(res->colorTable);
	if (res->bmpData) free(res->bmpData);
}

