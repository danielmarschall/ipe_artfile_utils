/**
 * ART file packer by Daniel Marschall, ViaThinkSoft (C) 2014-2018
 * Supports:
 * - Where's Waldo? Exploring Geography
 * - Eraser Turnabout by Imagination Pilots
 * - Virtual K'Nex by Imagination Pilots
 * Revision: 2018-02-15
 **/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "ipe_artfile_packer_ipe32.h"
#include "ipe32_artfile.h"
#include "ipe32_bmpimport.h"
#include "ipe32_lzw_encoder.h"

#define MAX_FILE 256

bool ipe32_pack_art(const char* szSrcFolder, FILE* fobArt, const int verbosity) {
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

	Ipe32FileHeader efh;
	memset(&efh, 0x00, sizeof(efh));
	strcpy(efh.magic, IPE32_MAGIC_ART);
	efh.reserved = 0;
	efh.totalHeaderSize = (cItems+1)*sizeof(efh);

	Ipe32PictureEntryHeader peh[cItems];
	memset(&peh, 0x00, sizeof(peh));

	// These headers are currently just dummies. They will be rewritten after all pictures are processed
	fwrite(&efh, sizeof(efh), 1, fobArt);
	fwrite(&peh, sizeof(peh), 1, fobArt);

	Ipe32LZWEncoder *encoder = new_ipe32lzw_encoder();
	ipe32lzw_init_encoder(encoder);
	fseek(fitIndex, 0, SEEK_SET);
	int curItem = 0;
	while (fgets(line, sizeof(line), fitIndex)) {
		// If something fails, we discard the item, but continue in building the file!
		#define FAIL_CONTINUE { memset(&peh[curItem], 0x00, sizeof(peh[curItem])); bEverythingOK=false; continue; }

		const char* szDelimiters = " \t\r\n";
		char* szName                = strtok(&line[0], szDelimiters);
		/* char* szNumCompressedChunks = */ strtok(NULL, szDelimiters);
		/* char* szNumRawChunks        = */ strtok(NULL, szDelimiters);
		char* szFilename            = strtok(NULL, szDelimiters);

		if (strlen(szName) > IPE32_NAME_SIZE) {
			fprintf(stderr, "ERROR: Name %s is too long (max %d chars allowed)\n", szName, IPE32_NAME_SIZE);
			FAIL_CONTINUE;
		}

		char szBitmapFilename[MAX_FILE];
		sprintf(szBitmapFilename, "%s/%s", szSrcFolder, szFilename);
		FILE* fibBitmap = fopen(szBitmapFilename, "rb");
		if (!fibBitmap) {
			fprintf(stderr, "ERROR: cannot open '%s'\n", szFilename);
			FAIL_CONTINUE;
		}

		Ipe32BmpImportData result={0};
		if (!ipe32_bmp_import(fibBitmap, &result)) { // This function moves the file pointer to the bitmap info header
			fprintf(stderr, "Error at %s: %s\n", szFilename, result.error);
			fclose(fibBitmap);
			ipe32_free_bmpimport_result(&result);
			FAIL_CONTINUE;
		}

		strcpy(peh[curItem].name, szName);
		peh[curItem].offset = ftell(fobArt);
		peh[curItem].uncompressedSize = result.dataSize;
		if (verbosity >= 1) printf("Process %s at offset %x\n", szName, peh[curItem].offset);

		// Now write the chunks

		int chunkNo = 0;
		unsigned char uncompressedChunk[0x3FFE];
		unsigned char compressedChunk[0x3FFE];
		while (1) {
			if (verbosity >= 2) fprintf(stdout, "Bitmap %s: Write chunk %d.\n", szFilename, chunkNo);

			int uncompressedSize = fread(uncompressedChunk, 1, sizeof(uncompressedChunk), fibBitmap);
			if (uncompressedSize == 0) break; // done

			int compressedSize = ipe32lzw_encode(encoder, compressedChunk, sizeof(compressedChunk), uncompressedChunk, uncompressedSize);

			uint16_t len;

			if ((compressedSize == -1) || (compressedSize >= uncompressedSize)) {
				// Choose uncompressed chunk
				len = 0x8000 | uncompressedSize;
				fwrite(&len, sizeof(len), 1, fobArt);
				fwrite(uncompressedChunk, uncompressedSize, 1, fobArt);
			} else {
				// Choose compressed chunk
				len = compressedSize;
				fwrite(&len, sizeof(len), 1, fobArt);
				fwrite(compressedChunk, compressedSize, 1, fobArt);
			}

			chunkNo++;
		}

		// Free and continue

		fclose(fibBitmap);
		ipe32_free_bmpimport_result(&result);

		++curItem;
	}
	fclose(fitIndex);
	ipe32lzw_free_encoder(encoder);

	fseek(fobArt, 0, SEEK_SET);
	fwrite(&efh, sizeof(efh), 1, fobArt);
	fwrite(&peh, sizeof(peh), 1, fobArt);

	fclose(fobArt);

	return bEverythingOK;
}
