/**
 * LZW Decoder for Imagination Pilots Entertainment 16-bit games (IPE16)
 * - Blown Away - The Interactive Game by Imagination Pilots (BA)
 * - Panic in the Park - The Interactive Game by Imagination Pilots (PiP)
 * - Where's Waldo? At the Circus (Waldo1)
 * ART file packer and unpacker by Daniel Marschall, ViaThinkSoft (C) 2014-2018
 * Revision: 2018-02-15
 *
 * The code is based on "Cross platform GIF source code" (c) L. Patrick
 * http://www.cs.usyd.edu.au/~graphapp/package/src/libgif/gif.c
 * It was simplified and modified to encode IPE16-LZW instead of GIF-LZW.
 * The game uses exactly the compressed stream as defined in the GIF standard,
 * but the compressed stream is not divided into chunks.
 **/

#ifndef __inc__ipe16_lzw_decoder
#define __inc__ipe16_lzw_decoder

#include <stdio.h>
#include <stdbool.h>

#define LZ_MIN_BITS     9
#define LZ_MAX_BITS     12

#define LZ_MAX_CODE     4095    /* Largest 12 bit code */
#define NO_SUCH_CODE    4098    /* Impossible code = empty */

#define CLEAR_CODE      256
#define END_CODE        257
#define FIRST_CODE      258

typedef struct tagIpe16LZWDecoder {
    int running_code;
	int running_bits;
	int max_code_plus_one;
    int shift_state;
    unsigned long shift_data;
    unsigned char stack[LZ_MAX_CODE+1];
    unsigned int  suffix[LZ_MAX_CODE+1];
    unsigned int  prefix[LZ_MAX_CODE+1];
  } Ipe16LZWDecoder;

Ipe16LZWDecoder* new_ipe16lzw_decoder(void);
void del_ipe16lzw_decoder(Ipe16LZWDecoder* decoder);
/*unsigned*/ int ipe16lzw_decode(FILE* outFile, Ipe16LZWDecoder *decoder, unsigned char *input, int inputLength);

#endif // #ifndef __inc__ipe16_lzw_decoder

