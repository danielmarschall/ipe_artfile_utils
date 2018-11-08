/**
 * LZW Encoder for Imagination Pilots Entertainment 16-bit games (IPE16)
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

#ifndef __inc__ipe16_lzw_encoder
#define __inc__ipe16_lzw_encoder

#include <stdio.h>
#include <stdbool.h>

#define LZ_MIN_BITS     9

#define LZ_MAX_CODE     4095    /* Largest 12 bit code */
#define FLUSH_OUTPUT    4096    /* Impossible code = flush */

#define HT_SIZE         8192    /* 13 bit hash table size */
#define HT_KEY_MASK     0x1FFF  /* 13 bit key mask */

#define CLEAR_CODE      256
#define END_CODE        257
#define FIRST_CODE      258

#define HT_GET_KEY(x)	((x) >> 12)
#define HT_GET_CODE(x)	((x) & 0x0FFF)
#define HT_PUT_KEY(x)	((x) << 12)
#define HT_PUT_CODE(x)	((x) & 0x0FFF)

typedef struct tagIpe16LZWEncoder {
    int running_code;
	int running_bits;
    int max_code_plus_one;
	int shift_state;
    unsigned long shift_data;
    unsigned long hash_table[HT_SIZE];
  } Ipe16LZWEncoder;

Ipe16LZWEncoder* new_ipe16lzw_encoder(void);
void del_ipe16lzw_encoder(Ipe16LZWEncoder* encoder);
void ipe16lzw_encode(FILE* outFile, Ipe16LZWEncoder* encoder, unsigned char* input, int inputLength);

#endif // #ifndef __inc__ipe16_lzw_encoder

