/**
 * ART file packer by Daniel Marschall, ViaThinkSoft (C) 2014-2018
 * Supports:
 * - Panic in the Park - The Interactive Game by Imagination Pilots
 * - Where's Waldo? At the Circus (Waldo1)
 * Revision: 2018-02-15
 **/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "ipe_artfile_packer_ipe16_pip.h"
#include "ipe16_artfile.h"
#include "ipe16_bmpimport.h"
#include "ipe16_lzw_encoder.h"

#define MAX_FILE 256

bool pip_pack_art(const char* szSrcFolder, FILE* fobArt, const int verbosity) {
	bool bEverythingOK = true;

	char szIndexFilename[MAX_FILE];
	sprintf(szIndexFilename, "%s/index.txt", szSrcFolder);
	FILE* fitIndex = fopen(szIndexFilename, "rt");
	if (!fitIndex) {
		fprintf(stderr, "Cannot open %s\n", szIndexFilename);
		return false;
	}

	#define MAX_LINE 1024
	char line[MAX_LINE];
	int cItems = 0;
	while (fgets(line, sizeof(line), fitIndex)) {
		if (strlen(line) == 0) continue;
		++cItems;
	}
	if (verbosity >= 1) printf("%s contains %d entries\n", szIndexFilename, cItems); // TODO: don't print double /

	Ipe16FileHeader bfh;
	memset(&bfh, 0x00, sizeof(bfh));
	strcpy(bfh.magic, IPE16_MAGIC_ART);
	bfh.dummy = IPE16_MAGIC_DUMMY;
	bfh.numHeaderEntries = cItems+1;

	Ipe16PictureEntryHeader peh[cItems];
	memset(&peh, 0x00, sizeof(peh));

	PipPictureHeader ph[cItems];
	memset(&ph,  0x00, sizeof(ph));

	// We need to write the (still empty) headers, so we can use ftell() to determine the offsets correctly
	// These headers are currently just dummies. They will be rewritten after all pictures are processed
	fwrite(&bfh, sizeof(bfh), 1, fobArt);
	fwrite(&peh, sizeof(peh), 1, fobArt);

	fseek(fitIndex, 0, SEEK_SET);
	int curItem = 0;
	Ipe16LZWEncoder* lzwEncoder = NULL;
	while (fgets(line, sizeof(line), fitIndex)) {
		// If something fails, we discard the item, but continue in building the file!
		#define FAIL_CONTINUE { memset(&peh[curItem], 0x00, sizeof(peh[curItem])); bEverythingOK=false; continue; }

		const char* szDelimiters = " \t\r\n";
		char* szPaletteType     = strtok(&line[0], szDelimiters);
		char* szCompressionType = strtok(NULL, szDelimiters);
		char* szName            = strtok(NULL, szDelimiters);
		char* szFilename        = strtok(NULL, szDelimiters);
		char* szOffsetX         = strtok(NULL, szDelimiters);
		int iOffsetX            = (szOffsetX != NULL) ? atoi(szOffsetX) : 0;
		char* szOffsetY         = strtok(NULL, szDelimiters);
		int iOffsetY            = (szOffsetY != NULL) ? atoi(szOffsetY) : 0;

		if (strlen(szPaletteType) != 1) {
			fprintf(stderr, "ERROR: Palette type (argument 1) at line %d is not valid (must be 1 char)\n", curItem+1);
			FAIL_CONTINUE;
		}
		const char chPaletteType = *szPaletteType;

		if ((chPaletteType != IPE16_PALETTETYPE_ATTACHED) && (chPaletteType != IPE16_PALETTETYPE_PARENT)) {
			fprintf(stderr, "ERROR: Unknown palette type '%c' at line %d\n", chPaletteType, curItem+1);
			FAIL_CONTINUE;
		}

		if (strlen(szCompressionType) != 1) {
			fprintf(stderr, "ERROR: Compression type (argument 2) at line %d is not valid (must be 1 char)\n", curItem+1);
			FAIL_CONTINUE;
		}
		const char chCompressionType = *szCompressionType;

		if (strlen(szName) > IPE16_NAME_SIZE) {
			fprintf(stderr, "ERROR: Name %s is too long (max %d chars allowed)\n", szName, IPE16_NAME_SIZE);
			FAIL_CONTINUE;
		}

		strcpy(peh[curItem].name, szName);
		peh[curItem].paletteType = chPaletteType;
		peh[curItem].offset = ftell(fobArt);
		peh[curItem].size = 0; // will be increased later

		if (verbosity >= 1) printf("Process %s at offset %x\n", szName, peh[curItem].offset);

		// Read bitmap

		const bool colorTableExisting = (chPaletteType == IPE16_PALETTETYPE_ATTACHED);

		char szBitmapFilename[MAX_FILE];
		sprintf(szBitmapFilename, "%s/%s", szSrcFolder, szFilename);
		FILE* fibBitmap = fopen(szBitmapFilename, "rb");
		if (!fibBitmap) {
			fprintf(stderr, "ERROR: cannot open '%s'\n", szFilename);
			FAIL_CONTINUE;
		}

		Ipe16BmpImportData result={0};
		if (!ipe16_bmp_import(fibBitmap, &result)) {
			fprintf(stderr, "Error at %s: %s\n", szFilename, result.error);
			fclose(fibBitmap);
			ipe16_free_bmpimport_result(&result);
			FAIL_CONTINUE;
		}

		ph[curItem].compressionType = chCompressionType;
		ph[curItem].offsetX = iOffsetX;
		ph[curItem].offsetY = iOffsetY;
		ph[curItem].width = result.width;
		ph[curItem].height = result.height;
		fwrite(&ph[curItem], sizeof(ph[curItem]), 1, fobArt);
		peh[curItem].size += sizeof(ph[curItem]);

		// Write picture data

		size_t tmpBefore = ftell(fobArt);
		if (chCompressionType == PIP_COMPRESSIONTYPE_LZW) {
			if (!lzwEncoder) lzwEncoder = new_ipe16lzw_encoder();
			ipe16lzw_encode(fobArt, lzwEncoder, result.bmpData, result.bmpDataSize);
		} else if (chCompressionType == PIP_COMPRESSIONTYPE_NONE) {
			fwrite(result.bmpData, result.bmpDataSize, 1, fobArt);
		} else {
			fprintf(stderr, "Unknown compression type '%c' at line %d\n", chCompressionType, curItem+1);
			fclose(fibBitmap);
			ipe16_free_bmpimport_result(&result);
			FAIL_CONTINUE;
		}
		peh[curItem].size += ftell(fobArt)-tmpBefore;

		if (colorTableExisting) {
			fwrite(result.colorTable, sizeof(*result.colorTable), 1, fobArt);
			peh[curItem].size += sizeof(*result.colorTable);
		}

		// Free and continue

		fclose(fibBitmap);
		ipe16_free_bmpimport_result(&result);

		++curItem;
	}
	if (lzwEncoder) del_ipe16lzw_encoder(lzwEncoder);
	fclose(fitIndex);

	bfh.totalFileSize = ftell(fobArt);

	fseek(fobArt, 0, SEEK_SET);
	fwrite(&bfh, sizeof(bfh), 1, fobArt);
	fwrite(&peh, sizeof(peh), 1, fobArt);

	fclose(fobArt);

	return bEverythingOK;
}
