/**
 * IPMA video frame extractor by Daniel Marschall, ViaThinkSoft (C) 2022
 * Supports codecs IPMA and IP20
 * Revision: 2022-01-16
 * License: Apache 2.0
 **/

#define VERSION "2022-01-16"

#define _CRT_SECURE_NO_WARNINGS
// #define VISUAL_STUDIO_TEST

#include <windows.h>
#include <vfw.h>
#include <stdio.h>
#include <stdbool.h>

#ifndef VISUAL_STUDIO_TEST
#include <getopt.h>
#endif

#define LZ_MIN_BITS     9
#define LZ_MAX_BITS     12

#define LZ_MAX_CODE     4095    /* Largest 12 bit code */
#define NO_SUCH_CODE    4098    /* Impossible code = empty */

#define CLEAR_CODE      256
#define END_CODE        257
#define FIRST_CODE      258

bool dirExists(const char* dirName_in) {
	DWORD ftyp = GetFileAttributesA(dirName_in);
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;  //something is wrong with your path!

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true;   // this is a directory!

	return false;    // this is not a directory!
}

typedef struct tagIpe16LZWDecoder {
	int running_code;
	int running_bits;
	int max_code_plus_one;
	int shift_state;
	unsigned long shift_data;
	unsigned char stack[LZ_MAX_CODE + 1];
	unsigned int  suffix[LZ_MAX_CODE + 1];
	unsigned int  prefix[LZ_MAX_CODE + 1];
} Ipe16LZWDecoder;

void ipe16lzw_init_decoder(Ipe16LZWDecoder* decoder) {
	decoder->running_code = FIRST_CODE;
	decoder->running_bits = LZ_MIN_BITS;
	decoder->max_code_plus_one = 1 << decoder->running_bits;
	decoder->shift_state = 0;
	decoder->shift_data = 0;

	int i;
	for (i = 0; i <= LZ_MAX_CODE; i++) {
		decoder->prefix[i] = NO_SUCH_CODE;
	}
}

int ipe16lzw_read_code(unsigned char** inFile, Ipe16LZWDecoder* decoder) {
	int code;
	unsigned char next_byte;
	static int code_masks[] = {
		0x0000, 0x0001, 0x0003, 0x0007,
		0x000f, 0x001f, 0x003f, 0x007f,
		0x00ff, 0x01ff, 0x03ff, 0x07ff,
		0x0fff
	};

	while (decoder->shift_state < decoder->running_bits) {
		next_byte = **inFile;
		*inFile = *inFile + 1;
		decoder->shift_data |=
			((unsigned long)next_byte) << decoder->shift_state;
		decoder->shift_state += 8;
	}

	code = decoder->shift_data & code_masks[decoder->running_bits];

	decoder->shift_data >>= decoder->running_bits;
	decoder->shift_state -= decoder->running_bits;

	if (++decoder->running_code > decoder->max_code_plus_one
		&& decoder->running_bits < LZ_MAX_BITS) {
		decoder->max_code_plus_one <<= 1;
		decoder->running_bits++;
	}

	return code;
}

static int ipe16lzw_trace_prefix(unsigned int* prefix, int code, int clear_code) {
	int i = 0;

	while (code > clear_code && i++ <= LZ_MAX_CODE) {
		code = prefix[code];
	}
	return code;
}

// Difference between ipma_lzw_decode and ipe16lzw_decode: At ipma_lzw_decode, inFile is "unsigned char**" and not "FILE*"
// We don't do unsigned, because we want to have <0 as error result
/*unsigned*/ int ipma_lzw_decode(unsigned char** inFile, Ipe16LZWDecoder* decoder, unsigned char* output, int outputLength) {
	int i = 0, j;
	int current_code;
	int current_prefix;
	int stack_ptr = 0;
	int prev_code = NO_SUCH_CODE;
	unsigned char* stack;
	unsigned int* prefix;
	unsigned int* suffix;
	unsigned int bytes_written = 0;

	ipe16lzw_init_decoder(decoder);

	prefix = decoder->prefix;
	suffix = decoder->suffix;
	stack = decoder->stack;

	/* Pop the stack */
	while (stack_ptr != 0 && i < outputLength) {
		output[i++] = stack[--stack_ptr];
		//if (i > bytes_written) bytes_written = i;
		++bytes_written;
	}

	while (i < outputLength) {
		current_code = ipe16lzw_read_code(inFile, decoder);

		if (current_code == END_CODE) {
			if (i != outputLength - 1) //  || decoder->pixel_count != 0
				return -1; /* unexpected eof */
			i++;
		}
		else if (current_code == CLEAR_CODE) {
			for (j = 0; j <= LZ_MAX_CODE; j++) {
				prefix[j] = NO_SUCH_CODE;
			}
			decoder->running_code = FIRST_CODE;
			decoder->running_bits = LZ_MIN_BITS;
			decoder->max_code_plus_one = 1 << decoder->running_bits;
			prev_code = NO_SUCH_CODE;
		}
		else {
			if (current_code < CLEAR_CODE) {
				output[i++] = current_code;
				//if (i > bytes_written) bytes_written = i;
				++bytes_written;
			}
			else {
				if ((current_code < 0) || (current_code > LZ_MAX_CODE))
					return -2; /* image defect */
				if (prefix[current_code] == NO_SUCH_CODE) {
					if (current_code == decoder->running_code - 2) {
						current_prefix = prev_code;
						suffix[decoder->running_code - 2]
							= stack[stack_ptr++]
							= ipe16lzw_trace_prefix(prefix, prev_code, CLEAR_CODE);
					}
					else {
						return -3; /* image defect */
					}
				}
				else {
					current_prefix = current_code;
				}
				j = 0;
				while (j++ <= LZ_MAX_CODE && current_prefix > CLEAR_CODE && current_prefix <= LZ_MAX_CODE) {
					stack[stack_ptr++] = suffix[current_prefix];
					current_prefix = prefix[current_prefix];
				}
				if (j >= LZ_MAX_CODE || current_prefix > LZ_MAX_CODE)
					return -4; /* image defect */

				stack[stack_ptr++] = current_prefix;

				while (stack_ptr != 0 && i < outputLength) {
					output[i++] = stack[--stack_ptr];
					//if (i > bytes_written) bytes_written = i;
					++bytes_written;
				}
			}
			if (prev_code != NO_SUCH_CODE) {
				if ((decoder->running_code < 2) ||
					(decoder->running_code > LZ_MAX_CODE + 2))
					return -5; /* image defect */
				prefix[decoder->running_code - 2] = prev_code;

				if (current_code == decoder->running_code - 2) {
					suffix[decoder->running_code - 2]
						= ipe16lzw_trace_prefix(prefix, prev_code, CLEAR_CODE);
				}
				else {
					suffix[decoder->running_code - 2]
						= ipe16lzw_trace_prefix(prefix, current_code, CLEAR_CODE);
				}
			}
			prev_code = current_code;
		}
	}

	return bytes_written;
}

#define BMP_LINE_PADDING 4
#define BI_SIGNATURE 0x4D42

// Difference between ipma_write_bmp and ipe16_write_bmp: At ipma_write_bmp, the imagedata is bottom-down, and the palette is a RGBA-structure and not a RGB-structure
void ipma_write_bmp(FILE* output, unsigned int width, unsigned int height, unsigned char* imagedata, size_t imagedata_len, RGBQUAD *pal, int numColors) {

	// Each line must be padded to a multiple of 4
	int pad = (BMP_LINE_PADDING - (width % BMP_LINE_PADDING)) % BMP_LINE_PADDING;
	int newwidth = width + pad;
	int newsize = newwidth * height;
	unsigned char* padded_imagedata = (unsigned char*)malloc(newsize);
	if (padded_imagedata == NULL) return;
	unsigned int i;
	for (i = 0; i < height; ++i) {
		int offset = newwidth * i;
		memcpy(&padded_imagedata[offset], imagedata, width);
		memset(&padded_imagedata[offset + width], 0, pad);
		imagedata += width;
	}

	BITMAPFILEHEADER bfh;
	bfh.bfType = BI_SIGNATURE;
	bfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * numColors + newsize;
	bfh.bfReserved1 = 0;
	bfh.bfReserved2 = 0;
	bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * numColors;
	fwrite(&bfh, sizeof(bfh), 1, output);

	BITMAPINFOHEADER bih;
	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biWidth = width;
	bih.biHeight = height; // (positive = "bottom-up"-Bitmap)
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
	fwrite(pal, sizeof(RGBQUAD) * numColors, 1, output);

	// Image data
	fwrite(padded_imagedata, newsize, 1, output);

	free(padded_imagedata);
}


typedef struct tagBitmapInfoAndPalette {
	BITMAPINFOHEADER bi;
	RGBQUAD  pal[256];
} BitmapInfoAndPalette;

bool ipma_export_frames_bmp(char* filename, char* outdir)
{
	PAVIFILE pFile;
	int res;
	PAVISTREAM pStream1;
	AVISTREAMINFOA asi1;

	// Try creating the directory
	CreateDirectoryA(outdir, NULL);

	if (!dirExists(outdir)) {
		fprintf(stderr, "ERROR: Directory couldn't be created! %s\n", outdir);
		return 1;
	}

	res = AVIFileOpenA(&pFile, filename, OF_SHARE_DENY_WRITE, 0L);
	if (res == AVIERR_FILEOPEN) {
		fprintf(stderr, "ERROR: AVIFileOpenA(%s) returns AVIERR_FILEOPEN. Does the file exist?\n", filename);
		return false;
	}
	if (res != 0) {
		fprintf(stderr, "ERROR: AVIFileOpenA(%s) returns %d\n", filename, res);
		return false;
	}

	res = AVIFileGetStream(pFile, &pStream1, streamtypeVIDEO, 0);
	if (res == AVIERR_NODATA) {
		fprintf(stderr, "ERROR: AVIFileGetStream returns AVIERR_NODATA\n");
		AVIFileRelease(pFile);
		return false;
	}
	if (res == AVIERR_MEMORY) {
		fprintf(stderr, "ERROR: AVIFileGetStream returns AVIERR_MEMORY\n");
		AVIFileRelease(pFile);
		return false;
	}
	if (res != 0) {
		fprintf(stderr, "ERROR: AVIFileGetStream returns %d\n", res);
		AVIFileRelease(pFile);
		return false;
	}


	res = AVIStreamInfoA(pStream1, &asi1, sizeof(asi1));
	if (res != 0) {
		fprintf(stderr, "ERROR: AVIStreamInfoA returns %d\n", res);
		AVIStreamRelease(pStream1);
		AVIFileRelease(pFile);
		return false;
	}


	// The official handler name is "ipma", but some AVI files also use "IPMA"
	int ipmaVersion = 0;
	//if (asi1.fccHandler == mmioFOURCC('i', 'p', 'm', 'a')) ipmaVersion = 1;
	if ((tolower(((asi1.fccHandler >> 0) & 0xFF)) == 'i') &&
		(tolower(((asi1.fccHandler >> 8) & 0xFF)) == 'p') &&
		(tolower(((asi1.fccHandler >> 16) & 0xFF)) == 'm') &&
		(tolower(((asi1.fccHandler >> 24) & 0xFF)) == 'a'))
	{
		ipmaVersion = 1;
	}

	// The official handler name is "IP20", but all AVI files use "ip20"
	//if (asi1.fccHandler == mmioFOURCC('I', 'P', '2', '0')) ipmaVersion = 2;
	if ((tolower(((asi1.fccHandler >> 0) & 0xFF)) == 'i') &&
		(tolower(((asi1.fccHandler >> 8) & 0xFF)) == 'p') &&
		(tolower(((asi1.fccHandler >> 16) & 0xFF)) == '2') &&
		(tolower(((asi1.fccHandler >> 24) & 0xFF)) == '0'))
	{
		ipmaVersion = 2;
	}

	if (ipmaVersion == 0) {
		fprintf(stderr, "ERROR: Not an IPMA or IP20 AVI file!\n");
		AVIStreamRelease(pStream1);
		AVIFileRelease(pFile);
		return false;
	}


	int framesWritten = 0;
	for (int i = 0; 1; i++) {
		BitmapInfoAndPalette* pstrf = (BitmapInfoAndPalette*)malloc(sizeof(BitmapInfoAndPalette));
		if (pstrf == NULL) return false;
		LONG strf_siz = sizeof(BitmapInfoAndPalette);
		ZeroMemory(pstrf, strf_siz);

		//res = pStream1->ReadFormat(i, (LPVOID)pstrf, &strf_siz);
		res = AVIStreamReadFormat(pStream1, i, (LPVOID)pstrf, &strf_siz);
		if (res != 0) {
			fprintf(stderr, "ERROR: Read format info failed\n");
			AVIStreamRelease(pStream1);
			AVIFileRelease(pFile);
			return false;
		}


		if (((ipmaVersion == 1) && (asi1.fccHandler != mmioFOURCC('I', 'p', 'm', 'a'))) &&
			((ipmaVersion == 2) && (asi1.fccHandler != mmioFOURCC('I', 'p', '2', '0'))))
		{
			// biCompression is case-sensitive and must be "Ipma" or "Ip20"
			if (ipmaVersion == 1) fprintf(stderr, "ERROR: biCompression is not Ipma!\n");
			if (ipmaVersion == 2) fprintf(stderr, "ERROR: biCompression is not Ip20!\n");
			AVIStreamRelease(pStream1);
			AVIFileRelease(pFile);
			return false;
		}

		// Note that for 2 files, bi.biSizeImage is wrong (much too small!)
		// LB05M08.AVI: biSizeImage (10598) != rectWidth * rectHeight (27492)
		// TY06M12.AVI: biSizeImage  (1274) != rectWidth * rectHeight  (8058)
		//int bufsiz_uncompressed = pstrf->bi.biSizeImage;
		int bufsiz_uncompressed = (asi1.rcFrame.right - asi1.rcFrame.left) * (asi1.rcFrame.bottom - asi1.rcFrame.top);
		
		// theoretically, compressed can sometimes be larger than uncompressed, so we multiply by 10
		int bufsiz_compressed = bufsiz_uncompressed * 10;

		unsigned char* buffer_uncompressed = (unsigned char*)malloc(bufsiz_uncompressed);
		if (buffer_uncompressed == NULL) return false;
		unsigned char* buffer_compressed = (unsigned char*)malloc(bufsiz_compressed);
		if (buffer_compressed == NULL) return false;


		LONG plBytes = 0;
		LONG plSamples = 0;
		//res = pStream1->Read(i, 1, buffer_compressed, bufsiz_compressed, &plBytes, &plSamples);
		res = AVIStreamRead(pStream1, i, 1, buffer_compressed, bufsiz_compressed, &plBytes, &plSamples);
		if ((res != 0) || (plSamples == 0)) break;

		int plBytesUncompressed;
		if ((plBytes == 0) && (plSamples > 0)) {
			// In "Panic in the Park", frames at the end of some videos have 0 bytes data.
			// This should be interpreted as "repeat frame" (still image). So we fill it with palette #0 (transparent)
			plBytesUncompressed = bufsiz_uncompressed;
			ZeroMemory(buffer_uncompressed, bufsiz_uncompressed);
		} else {
			Ipe16LZWDecoder* pdecoder = (Ipe16LZWDecoder*)malloc(sizeof(Ipe16LZWDecoder));
			if (pdecoder == NULL) return false;
			ZeroMemory(pdecoder, sizeof(Ipe16LZWDecoder));
			unsigned char* work_buffer_compressed = buffer_compressed;
			plBytesUncompressed = ipma_lzw_decode(&work_buffer_compressed, pdecoder, buffer_uncompressed, bufsiz_uncompressed);
			free(pdecoder);
		}
		if (plBytesUncompressed < 0) fprintf(stderr, "WARNING: LZW Error %d at frame %d\n", plBytesUncompressed, i);
		if (plBytesUncompressed != bufsiz_uncompressed) fprintf(stderr, "WARNING: piBytesUncompressed != bufsiz_uncompressed\n");
		if (plBytesUncompressed > 0) {
			char filnam[MAX_PATH];
			if (AVIStreamIsKeyFrame(pStream1, i)) {
				sprintf(filnam, "%s\\frame_%05d_key.bmp", outdir, i);
			} else {
				sprintf(filnam, "%s\\frame_%05d.bmp", outdir, i);
			}
			FILE* fh2 = fopen(filnam, "wb+");
			ipma_write_bmp(fh2, pstrf->bi.biWidth, pstrf->bi.biHeight, buffer_uncompressed, plBytesUncompressed, &pstrf->pal[0], 256);
			fclose(fh2);
			framesWritten++;
		}

		free(pstrf);
		free(buffer_compressed);
		free(buffer_uncompressed);
	}

	fprintf(stdout, "%s: %d frames written to %s\n", filename, framesWritten, outdir);

	AVIStreamRelease(pStream1);
	AVIFileRelease(pFile);
	return true;
}

void print_syntax() {
	fprintf(stderr, "Syntax: -o <outputdir> -i <avifile>\n");
}

int main(int argc, char* argv[]) {
	char filename[MAX_PATH];
	char outdir[MAX_PATH];

#ifndef VISUAL_STUDIO_TEST
	int c;

	#define PRINT_SYNTAX { print_syntax(); return 0; }

	while ((c = getopt(argc, argv, "Vi:o:")) != -1) {
		switch (c) {
		case 'V':
			fprintf(stdout, "IPMA video frame extractor, revision %s\n", VERSION);
			return 0;
		case 'i':
			strcpy(filename, optarg);
			break;
		case 'o':
			strcpy(outdir, optarg);
			break;
		case '?':
			PRINT_SYNTAX;
			break;
		}
	}
	if (optind < argc) PRINT_SYNTAX;

	if (strlen(filename) == 0) PRINT_SYNTAX;
	if (strlen(outdir) == 0) PRINT_SYNTAX;

	FILE* fhTest = fopen(filename, "rb");
	if (!fhTest) {
		fprintf(stderr, "FATAL: Cannot open %s\n", filename);
		return 1;
	}
	fclose(fhTest);

#else
	strcpy(filename, "D:\\test\\AVI_TEST.avi");
	strcpy(outdir, "D:\\test\\AVI_TEST");
#endif

	if (CoInitialize(NULL) != 0) return 1;

	bool res = ipma_export_frames_bmp(filename, outdir);

	CoUninitialize();

	return res ? 0 : 1;
}
