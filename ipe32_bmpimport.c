/**
 * Bitmap Import for Imagination Pilots Entertainment 32-bit games (IPE32)
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
#include <math.h>

#include "bitmap.h"
#include "utils.h"
#include "ipe32_bmpimport.h"

#define BI_BITFIELDS 3

bool bmp_has_gap1(BITMAPFILEHEADER bitmapFileHeader, BITMAPINFOHEADER bitmapInfoHeader) {
	int expected = 0;
	expected = sizeof(bitmapFileHeader) + bitmapInfoHeader.biSize;

	if (bitmapInfoHeader.biCompression == BI_BITFIELDS) {
		expected += 3*sizeof(DWORD);
	}

	int numColorEntries;
	if (bitmapInfoHeader.biClrUsed == 0) {
		if ((bitmapInfoHeader.biBitCount == 1) || (bitmapInfoHeader.biBitCount == 4) || (bitmapInfoHeader.biBitCount == 8)) {
			numColorEntries = pow(2, bitmapInfoHeader.biBitCount);
		} else {
			numColorEntries = 0;
		}
	} else {
		numColorEntries = bitmapInfoHeader.biClrUsed;
	}
	expected += sizeof(RGBQUAD)*numColorEntries;

	return bitmapFileHeader.bfOffBits != expected;
}

bool ipe32_bmp_import(FILE* fibBitmap, Ipe32BmpImportData* result) {
	BITMAPFILEHEADER bitmapFileHeader;
	BITMAPINFOHEADER bitmapInfoHeader;

	#define EXIT_ERROR(msg) { sprintf(result->error, msg); return false; }

	fseek(fibBitmap, 0, SEEK_SET);

	if (!fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, fibBitmap) ||
	    (bitmapFileHeader.bfType != BI_SIGNATURE) ||
	    !fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, fibBitmap)) {
		EXIT_ERROR("Not a bitmap file");
	}

	if (bmp_has_gap1(bitmapFileHeader, bitmapInfoHeader)) {
		EXIT_ERROR("Picture may not have a gap between header and bitmap data");
	}

	// TODO: Find out if the game has limitations in regards to compression, top-down/bottom-up, Bitmap version etc.
	/*
	if (bitmapInfoHeader.biCompression != BI_RGB) {
		EXIT_ERROR("At the moment, only uncompressed files can be read.");
	}

	if (bitmapInfoHeader.biBitCount != 8) {
		EXIT_ERROR("The color depth has to be 8 bpp.");
	}
	*/

	fseek(fibBitmap, sizeof(bitmapFileHeader), SEEK_SET);

	result->error[0] = 0;
	result->dataSize = file_size(fibBitmap)-sizeof(bitmapFileHeader);
	return true;
}

void ipe32_free_bmpimport_result(Ipe32BmpImportData *res) {
}

