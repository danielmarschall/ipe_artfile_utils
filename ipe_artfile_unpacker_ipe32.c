/**
 * ART file unpacker by Daniel Marschall, ViaThinkSoft (C) 2014-2018
 * Supports:
 * - Where's Waldo? Exploring Geography
 * - Eraser Turnabout by Imagination Pilots
 * - Virtual K'Nex by Imagination Pilots
 * Revision: 2018-02-15
 **/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <getopt.h>

#include "ipe32_bmpexport.h"
#include "ipe32_artfile.h"
#include "ipe32_lzw_decoder.h"

#include "utils.h"

#define MAX_FILE 256

typedef struct tagIpe32ReadPictureResult {
	uint32_t   writtenBytes;
	uint32_t   numCompressedChunks;
	uint32_t   numRawChunks;
} Ipe32ReadPictureResult;

Ipe32ReadPictureResult ipe32_read_picture(FILE* hFile, unsigned char* outbuf, const int outputBufLength, bool bVerbose) {
	unsigned char* lzwbuf = (unsigned char*)malloc(0x8000);
	int availableOutputBytes = outputBufLength;

	Ipe32ReadPictureResult res;
	res.numCompressedChunks = 0;
	res.numRawChunks = 0;
	res.writtenBytes = 0;

	Ipe32LZWDecoder *decoder = new_ipe32lzw_decoder();
	ipe32lzw_init_decoder(decoder);
	if (outputBufLength != 0) {
		int chunkNo = 0;
		do {
			uint16_t len;
			fread(&len, 1, 2, hFile);

			uint16_t writtenBytes;
			if (len < 0x8000) {
				fread(lzwbuf, 1, len, hFile);
				res.numCompressedChunks++;
				if (bVerbose) fprintf(stdout, "Chunk %d (compressed, length: %d) ...\n", chunkNo, len);

				// Requirement 1: Each chunk (except the last one) MUST have 0x3FFE of uncompressed data
				uint16_t expectedOutputSize = availableOutputBytes > 0x3FFE ? 0x3FFE : availableOutputBytes;

				// Requirement 2: The size of the uncompressed data must not exceed the size of the compressed data
				size_t maxReadBytes = expectedOutputSize;

				writtenBytes = ipe32lzw_decode(decoder, outbuf, outputBufLength, lzwbuf, maxReadBytes); // returns bytes written, or -1

				if (writtenBytes == -1) {
					fprintf(stderr, "ERROR: Fatal error during decompression of chunk %d!\n", chunkNo);
					break;
				}

				if (writtenBytes != expectedOutputSize) {
					fprintf(stderr, "ERROR: Chunk %d decompressed %d bytes, but %d bytes are expected!\n", chunkNo, writtenBytes, expectedOutputSize);
					break;
				}
			} else {
				len &= 0x7FFF;
				res.numRawChunks++;
				if (bVerbose) fprintf(stdout, "Chunk %d (raw, length: %d) ...\n", chunkNo, len);
				fread(outbuf, 1, len, hFile);
				writtenBytes = len;
			}
			outbuf += writtenBytes;
			availableOutputBytes -= writtenBytes;
			chunkNo++;
		} while (availableOutputBytes != 0);
	}
	ipe32lzw_free_decoder(decoder);

	free(lzwbuf);
	res.writtenBytes = outputBufLength-availableOutputBytes;
	return res;
}

bool ipe32_extract_art_to_folder(FILE* fibArt, const char* szDestFolder, const int verbosity) {
	bool bEverythingOK = true;

	fseek(fibArt, 0, SEEK_SET);

	Ipe32FileHeader efh;
	if (fread(&efh, sizeof(efh), 1, fibArt) != 1) {
		fprintf(stderr, "FATAL: Cannot read Ipe32FileHeader. It is probably not an art file.\n");
		return false;
	}

	// Check if the super header is correct
	if ((memcmp(efh.magic, IPE32_MAGIC_ART, 8) != 0) || (efh.reserved != 0)) {
		fprintf(stderr, "FATAL: Something does not seem to be correct with this art file's header. It is probably not an art file.\n");
		return false;
	}

	FILE* fotIndex = NULL;
	if (strlen(szDestFolder) > 0) {
		char szIndexFilename[MAX_FILE];
		sprintf(szIndexFilename, "%s/index.txt", szDestFolder);
		fotIndex = fopen(szIndexFilename, "wt");
		if (!fotIndex) {
			fprintf(stderr, "FATAL: Cannot open %s for writing\n", szIndexFilename);
			return false;
		}
	}

	const int numPictures = efh.totalHeaderSize/sizeof(efh) - 1;
	char knownNames[numPictures][IPE32_NAME_SIZE];
	memset(knownNames, 0, numPictures*IPE32_NAME_SIZE);
	int iPicNo;
	for (iPicNo=0; iPicNo<numPictures; ++iPicNo) {
		Ipe32PictureEntryHeader peh;
		if (fread(&peh, sizeof(peh), 1, fibArt) != 1) {
			fprintf(stderr, "FATAL: Cannot read Ipe32PictureEntryHeader.\n");
			return false;
		}

		char szName[IPE32_NAME_SIZE+1]={0};
		memcpy(szName, peh.name, IPE32_NAME_SIZE);

		// Begin duplicate check
		// In ERASER, there are a few pictures which have the same identifier, in the same ART file!
		memcpy(&knownNames[iPicNo][0], peh.name, IPE32_NAME_SIZE);
		int iCopyNumber = 0;
		int j;
		for (j=0; j<=iPicNo; ++j) {
			if (memcmp(&knownNames[j][0], peh.name, IPE32_NAME_SIZE) == 0) ++iCopyNumber;
		}
		assert(iCopyNumber > 0);
		// End duplicate check

		if (verbosity >= 2) fprintf(stdout, "Extracting %s (expected file size: %d bytes) ...\n", szName, peh.uncompressedSize);

		size_t headersPos = ftell(fibArt);
		#define FAIL_CONTINUE { bEverythingOK = false; fseek(fibArt, headersPos, SEEK_SET); continue; }

		if (fseek(fibArt, peh.offset, SEEK_SET) != 0) {
			fprintf(stderr, "ERROR: Error jumping to offset defined for %s\n", szName);
			FAIL_CONTINUE;
		}

		int outputBufLen = peh.uncompressedSize;
		unsigned char* outputBuf = (unsigned char*)malloc(outputBufLen);

		char szBitmapFilename[MAX_FILE];
		if (iCopyNumber == 1) {
			sprintf(szBitmapFilename, "%s.bmp", sanitize_filename(szName));
		} else {
			sprintf(szBitmapFilename, "%s__%d.bmp", sanitize_filename(szName), iCopyNumber);
		}

		Ipe32ReadPictureResult res = ipe32_read_picture(fibArt, outputBuf, outputBufLen, verbosity >= 2);
		if (res.writtenBytes != outputBufLen) {
			fprintf(stderr, "FATAL: Error reading picture %s (compression failure?)\n", szName);
			FAIL_CONTINUE;
		}

		if (strlen(szDestFolder) > 0) {
			char szAbsoluteBitmapFilename[MAX_FILE+1];
			sprintf(szAbsoluteBitmapFilename, "%s/%s", szDestFolder, szBitmapFilename);
			FILE* fobBitmap = fopen(szAbsoluteBitmapFilename, "wb");
			if (!fobBitmap) {
				fprintf(stderr, "FATAL: Cannot open %s for writing\n", szAbsoluteBitmapFilename);
				FAIL_CONTINUE;
			}
			ipe32_write_bmp(fobBitmap, outputBuf, outputBufLen);
			fclose(fobBitmap);
		}

		free(outputBuf);

		if (fotIndex) {
			// We require this index file so that our packer tool can know what to pack
			// The index file won't be written in simulation mode (when no output directory is defined)
			fprintf(fotIndex, "%s %d(C) %d(R) %s\n", szName, res.numCompressedChunks, res.numRawChunks, szBitmapFilename);
		}
		if (verbosity >= 1) {
			fprintf(stdout, "%s %d(C) %d(R) %s\n", szName, res.numCompressedChunks, res.numRawChunks, szBitmapFilename);
		}

		fseek(fibArt, headersPos, SEEK_SET); // we were already there, so we don't need to check for errors
	}

	if (strlen(szDestFolder) > 0) fclose(fotIndex);

	return bEverythingOK;
}
