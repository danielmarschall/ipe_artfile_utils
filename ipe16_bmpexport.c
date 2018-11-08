/**
 * Bitmap Export for Imagination Pilots Entertainment 16-bit games (IPE16)
 * - Blown Away - The Interactive Game by Imagination Pilots (BA)
 * - Panic in the Park - The Interactive Game by Imagination Pilots (PiP)
 * - Where's Waldo? At the Circus (Waldo1)
 * ART file packer and unpacker by Daniel Marschall, ViaThinkSoft (C) 2014-2018
 * Revision: 2018-02-15
 **/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ipe16_bmpexport.h"

#define BMP_LINE_PADDING 4

// Windows 98 does not support top-down bitmaps, so we need to flip everything
#define USE_BOTTOMUP

RGBQUAD ipe16_rgb_to_rgbquad(Ipe16ColorTableEntry cte) {
	RGBQUAD ret;
	ret.rgbRed   = cte.r;
	ret.rgbGreen = cte.g;
	ret.rgbBlue  = cte.b;
	ret.rgbReserved = 0;
	return ret;
}

void ipe16_write_bmp(FILE* output, unsigned int width, unsigned int height, unsigned char* imagedata, size_t imagedata_len, Ipe16ColorTable ct) {

	#ifdef USE_BOTTOMUP
	const size_t bmpDataSize = width*height;
	unsigned char* bmpData = (unsigned char*)malloc(bmpDataSize);
	assert(bmpData != NULL);
	int h;
	for (h=0; h<height; ++h) {
		int idx_dest = (height-1)-h;
		int idx_src  = h;
		assert(idx_src*width >= 0);
		assert(idx_dest*width >= 0);
		memcpy(bmpData+idx_dest*width, imagedata+idx_src*width, width);
	}
	imagedata = bmpData;
	#endif

	// Each line must be padded to a multiple of 4
	int pad = (BMP_LINE_PADDING - (width % BMP_LINE_PADDING)) % BMP_LINE_PADDING;
	int newwidth = width+pad;
	int newsize = newwidth * height;
	unsigned char* padded_imagedata = (unsigned char*)malloc(newsize);
	int i;
	for (i=0; i<height; ++i) {
		int offset = newwidth*i;
		memcpy(&padded_imagedata[offset], imagedata, width);
		memset(&padded_imagedata[offset+width], 0, pad);
		imagedata += width;
	}

	// Color table in a bitmap is BGR0, while Blown Away uses RGB
	const unsigned int NUM_COLORS = sizeof(ct.colors)/sizeof(ct.colors[0]);
	RGBQUAD rgba_colortable[NUM_COLORS];
	for (i=0; i<NUM_COLORS; ++i) {
		rgba_colortable[i] = ipe16_rgb_to_rgbquad(ct.colors[i]);
	}

	BITMAPFILEHEADER bfh;
	bfh.bfType = BI_SIGNATURE;
	bfh.bfSize = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+sizeof(rgba_colortable)+newsize;
	bfh.bfReserved1 = 0;
	bfh.bfReserved2 = 0;
	bfh.bfOffBits = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+sizeof(rgba_colortable);
	fwrite(&bfh, sizeof(bfh), 1, output);

	BITMAPINFOHEADER bih;
	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biWidth = width;
	#ifdef USE_BOTTOMUP
	bih.biHeight = height; // (positive = "bottom-up"-Bitmap)
	#else
	bih.biHeight = -height; // (negative = "top-down"-Bitmap)
	#endif
	bih.biPlanes = 1;
	bih.biBitCount = 8;
	bih.biCompression = BI_RGB;
	bih.biSizeImage = 0;
	bih.biXPelsPerMeter = 0;
	bih.biYPelsPerMeter = 0;
	bih.biClrUsed = 0;
	bih.biClrImportant = 0;
	fwrite(&bih, sizeof(bih), 1, output);

	// Color table
	fwrite(rgba_colortable, sizeof(rgba_colortable), 1, output);

	// Image data
	fwrite(padded_imagedata, newsize, 1, output);

	free(padded_imagedata);
	#ifdef USE_BOTTOMUP
	free(bmpData);
	#endif
}

