/**
 * ART file unpacker by Daniel Marschall, ViaThinkSoft (C) 2014-2018
 * Supports:
 * - Blown Away - The Interactive Game by Imagination Pilots
 * - Panic in the Park - The Interactive Game by Imagination Pilots
 * - Where's Waldo? At the Circus (Waldo1)
 * Revision: 2018-02-15
 **/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <getopt.h>

#include "ipe16_bmpexport.h"
#include "ipe16_artfile.h"
#include "ipe16_lzw_decoder.h"

#include "utils.h"

#define MAX_FILE 256

void ipe16_generate_gray_table(Ipe16ColorTable *ct) {
	int i;
	for (i=0; i<=0xFF; ++i) {
		ct->colors[i].r = i;
		ct->colors[i].g = i;
		ct->colors[i].b = i;
	}
}

bool ipe16_extract_art_to_folder(FILE* fibArt, const char* szDestFolder, const int verbosity) {
	bool bEverythingOK = true;

	fseek(fibArt, 0, SEEK_SET);

	Ipe16FileHeader bfh;
	if (fread(&bfh, sizeof(bfh), 1, fibArt) != 1) {
		fprintf(stderr, "FATAL: Cannot read Ipe16FileHeader. It is probably not an art file.\n");
		return false;
	}

	// The "super header" has some different meanings of the fields
	// Name and Type are hardcoded
	// startOffset is the number of header entries (including the super header)
	// length is the complete file size
	const size_t fileSize = file_size(fibArt);
	if ((strcmp(bfh.magic, IPE16_MAGIC_ART) != 0) || // better memcpy over all 23 bytes?
		(bfh.dummy != IPE16_MAGIC_DUMMY) ||
		(bfh.totalFileSize != fileSize)) {
		fprintf(stderr, "FATAL: Something does not seem to be correct with this art file's header. It is probably not an art file.\n");
		return false;
	}

	Ipe16LZWDecoder* lzwDecoder = NULL;

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

	const int numPictures = bfh.numHeaderEntries - 1;
	char knownNames[numPictures][IPE16_NAME_SIZE];
	memset(&knownNames[0][0], 0, numPictures*IPE16_NAME_SIZE);
	int iPicNo;
	for (iPicNo=0; iPicNo<numPictures; ++iPicNo) {
		Ipe16PictureEntryHeader peh;
		if (fread(&peh, sizeof(peh), 1, fibArt) != 1) {
			fprintf(stderr, "FATAL: Cannot read Ipe16PictureEntryHeader.\n");
			return false;
		}

		// Begin duplicate check
		memcpy(&knownNames[iPicNo][0], peh.name, IPE16_NAME_SIZE);
		int iCopyNumber = 0;
		int j;
		for (j=0; j<=iPicNo; ++j) {
			// TODO: should we rather use strcmp() in IPE16?
			if (memcmp(&knownNames[j][0], peh.name, IPE16_NAME_SIZE) == 0) ++iCopyNumber;
		}
		assert(iCopyNumber > 0);
		// End duplicate check

		// in the English version of Blown Away (not the Special Edition), there is a header entry
		// with the fields seh.name='', peh.paletteType=0x00, seh.offset[end of file], seh.size=0
		// ignore it
		if (strlen(peh.name) == 0) continue;
		if (peh.size == 0) continue;

		if (strlen(peh.name) > IPE16_NAME_SIZE-1) {
			fprintf(stderr, "FATAL: szName at picture %d is breaking the boundaries. The file is probably corrupt.\n", iPicNo);
			return false;
		}

		size_t headersPos = ftell(fibArt);
		#define FAIL_CONTINUE { bEverythingOK = false; fseek(fibArt, headersPos, SEEK_SET); continue; }

		if (fseek(fibArt, peh.offset, SEEK_SET) != 0) {
			fprintf(stderr, "ERROR: Error jumping to offset defined for %s\n", peh.name);
			FAIL_CONTINUE;
		}

		if (peh.offset+peh.size > fileSize) {
			fprintf(stderr, "ERROR: Defined size of %s exceeds file size\n", peh.name);
			FAIL_CONTINUE;
		}

		unsigned char compressionType;
		int bakPos = ftell(fibArt);
		fread(&compressionType, sizeof(compressionType), 1, fibArt);
		fseek(fibArt, bakPos, SEEK_SET);

		if ((compressionType == BA_COMPRESSIONTYPE_LZW) || (compressionType == BA_COMPRESSIONTYPE_NONE)) {
			BAPictureHeader ph;
			if (fread(&ph, sizeof(ph), 1, fibArt) != 1) {
				fprintf(stderr, "ERROR: Cannot read BAPictureHeader of %s\n", peh.name);
				FAIL_CONTINUE;
			}

			size_t imagedata_len = ph.width * ph.height;
			unsigned char* imagedata = (unsigned char*)malloc(imagedata_len);

			Ipe16ColorTable ct;
			if (peh.paletteType == IPE16_PALETTETYPE_ATTACHED) {
				if ((fseek(fibArt, peh.offset+peh.size-sizeof(ct), SEEK_SET) != 0) ||
				    (fread(&ct, sizeof(ct), 1, fibArt) != 1) ||
				    (fseek(fibArt, peh.offset+sizeof(ph), SEEK_SET) != 0)) {
					fprintf(stderr, "ERROR: Cannot read palette of %s\n", peh.name);
					FAIL_CONTINUE;
				}
			} else if (peh.paletteType == IPE16_PALETTETYPE_PARENT) {
				ipe16_generate_gray_table(&ct);
			} else {
				fprintf(stderr, "ERROR: Unknown palette type 0x%x at %s\n", peh.paletteType, peh.name);
				FAIL_CONTINUE;
			}

			unsigned int bytes_written, expected_uncompressed_len;
			switch (ph.compressionType) {
				case BA_COMPRESSIONTYPE_LZW:
					if (!lzwDecoder) lzwDecoder = new_ipe16lzw_decoder();
					bytes_written = ipe16lzw_decode(fibArt, lzwDecoder, imagedata, imagedata_len);
					if (bytes_written < 0) {
						fprintf(stderr, "ERROR: LZW decompression error at %s\n", peh.name);
						FAIL_CONTINUE;
					}
					if (bytes_written != imagedata_len) {
						fprintf(stderr, "ERROR: Image dimensions and decompressed data size does not match for %s\n", peh.name);
						FAIL_CONTINUE;
					}

					break;
				case BA_COMPRESSIONTYPE_NONE:
					expected_uncompressed_len = imagedata_len;
					if (peh.paletteType == IPE16_PALETTETYPE_ATTACHED) expected_uncompressed_len += sizeof(ct);
					expected_uncompressed_len += sizeof(ph.compressionType)+sizeof(ph.width)+sizeof(ph.height);
					if (expected_uncompressed_len != peh.size) {
						fprintf(stderr, "ERROR: Image dimensions/palette (%d) and defined memory size (%d) does not match for %s\n", expected_uncompressed_len, peh.size, peh.name);
						FAIL_CONTINUE;
					}
					fread(imagedata, imagedata_len, 1, fibArt); // no error checking, because filesize was already checked
					break;
			}

			char szBitmapFilename[MAX_FILE];
			if (iCopyNumber == 1) {
				sprintf(szBitmapFilename, "%s.bmp", sanitize_filename(peh.name));
			} else {
				sprintf(szBitmapFilename, "%s__%d.bmp", sanitize_filename(peh.name), iCopyNumber);
			}

			if (strlen(szDestFolder) > 0) {
				char szAbsoluteBitmapFilename[MAX_FILE];
				sprintf(szAbsoluteBitmapFilename, "%s/%s", szDestFolder, szBitmapFilename);
				FILE* fobBitmap = fopen(szAbsoluteBitmapFilename, "wb");
				if (!fobBitmap) {
					fprintf(stderr, "FATAL: Cannot open %s for writing\n", szAbsoluteBitmapFilename);
					FAIL_CONTINUE;
				}
				ipe16_write_bmp(fobBitmap, ph.width, ph.height, imagedata, imagedata_len, ct);
				fclose(fobBitmap);
			}

			if (fotIndex) {
				// We require this index file for 2 reasons
				// 1. Our packer tool can then know what to pack
				// 2. The packer tool can know which picture would have a palette appended and which one does not
				// The index file won't be written in simulation mode (when no output directory is defined)
				fprintf(fotIndex, "%c %c %s %s\n", peh.paletteType, ph.compressionType, peh.name, szBitmapFilename);
			}
			if (verbosity >= 1) {
				fprintf(stdout, "%c %c %s %s\n", peh.paletteType, ph.compressionType, peh.name, szBitmapFilename);
			}

			free(imagedata);
		} else if ((compressionType == PIP_COMPRESSIONTYPE_LZW) || (compressionType == PIP_COMPRESSIONTYPE_NONE)) {
			PipPictureHeader ph;
			if (fread(&ph, sizeof(ph), 1, fibArt) != 1) {
				fprintf(stderr, "ERROR: Cannot read PipPictureHeader of %s\n", peh.name);
				FAIL_CONTINUE;
			}

			size_t imagedata_len = ph.width * ph.height;
			unsigned char* imagedata = (unsigned char*)malloc(imagedata_len);

			Ipe16ColorTable ct;
			if (peh.paletteType == IPE16_PALETTETYPE_ATTACHED) {
				if ((fseek(fibArt, peh.offset+peh.size-sizeof(ct), SEEK_SET) != 0) ||
				    (fread(&ct, sizeof(ct), 1, fibArt) != 1) ||
				    (fseek(fibArt, peh.offset+sizeof(ph), SEEK_SET) != 0)) {
					fprintf(stderr, "ERROR: Cannot read palette of %s\n", peh.name);
					FAIL_CONTINUE;
				}
			} else if (peh.paletteType == IPE16_PALETTETYPE_PARENT) {
				ipe16_generate_gray_table(&ct);
			} else {
				fprintf(stderr, "ERROR: Unknown palette type 0x%x at %s\n", peh.paletteType, peh.name);
				FAIL_CONTINUE;
			}

			unsigned int bytes_written, expected_uncompressed_len;
			switch (ph.compressionType) {
				case PIP_COMPRESSIONTYPE_LZW:
					if (!lzwDecoder) lzwDecoder = new_ipe16lzw_decoder();
					bytes_written = ipe16lzw_decode(fibArt, lzwDecoder, imagedata, imagedata_len);
					if (bytes_written < 0) {
						fprintf(stderr, "ERROR: LZW decompression error at %s\n", peh.name);
						FAIL_CONTINUE;
					}
					if (bytes_written != imagedata_len) {
						fprintf(stderr, "ERROR: Image dimensions and decompressed data size does not match for %s\n", peh.name);
						FAIL_CONTINUE;
					}

					break;
				case PIP_COMPRESSIONTYPE_NONE:
					expected_uncompressed_len = imagedata_len;
					if (peh.paletteType == IPE16_PALETTETYPE_ATTACHED) expected_uncompressed_len += sizeof(ct);
					expected_uncompressed_len += sizeof(ph.compressionType)+sizeof(ph.offsetX)+sizeof(ph.offsetY)+sizeof(ph.width)+sizeof(ph.height);
					if (expected_uncompressed_len != peh.size) {
						fprintf(stderr, "ERROR: Image dimensions/palette (%d) and defined memory size (%d) does not match for %s\n", expected_uncompressed_len, peh.size, peh.name);
						FAIL_CONTINUE;
					}
					fread(imagedata, imagedata_len, 1, fibArt); // no error checking, because filesize was already checked
					break;
			}

			char szBitmapFilename[MAX_FILE];
			if (iCopyNumber == 1) {
				sprintf(szBitmapFilename, "%s.bmp", sanitize_filename(peh.name));
			} else {
				sprintf(szBitmapFilename, "%s__%d.bmp", sanitize_filename(peh.name), iCopyNumber);
			}

			if (strlen(szDestFolder) > 0) {
				char szAbsoluteBitmapFilename[MAX_FILE];
				sprintf(szAbsoluteBitmapFilename, "%s/%s", szDestFolder, szBitmapFilename);
				FILE* fobBitmap = fopen(szAbsoluteBitmapFilename, "wb");
				if (!fobBitmap) {
					fprintf(stderr, "FATAL: Cannot open %s for writing\n", szAbsoluteBitmapFilename);
					FAIL_CONTINUE;
				}
				ipe16_write_bmp(fobBitmap, ph.width, ph.height, imagedata, imagedata_len, ct);
				fclose(fobBitmap);
			}

			if (fotIndex) {
				// We require this index file for 2 reasons
				// 1. Our packer tool can then know what to pack
				// 2. The packer tool can know which picture would have a palette appended and which one does not
				// The index file won't be written in simulation mode (when no output directory is defined)
				fprintf(fotIndex, "%c %c %s %s %d %d\n", peh.paletteType, ph.compressionType, peh.name, szBitmapFilename, ph.offsetX, ph.offsetY);
			}
			if (verbosity >= 1) {
				fprintf(stdout, "%c %c %s %s %d %d\n", peh.paletteType, ph.compressionType, peh.name, szBitmapFilename, ph.offsetX, ph.offsetY);
			}

			free(imagedata);
		} else {
			fprintf(stderr, "ERROR: Unknown compression type 0x%x at %s\n", compressionType, peh.name);
			FAIL_CONTINUE;
		}

		fseek(fibArt, headersPos, SEEK_SET); // we were already there, so we don't need to check for errors
	}

	if (lzwDecoder) del_ipe16lzw_decoder(lzwDecoder);

	if (strlen(szDestFolder) > 0) fclose(fotIndex);

	return bEverythingOK;
}
