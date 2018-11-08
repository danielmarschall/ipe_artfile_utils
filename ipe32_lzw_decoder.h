/**
 * LZW Decoder for Imagination Pilots Entertainment 32-bit games (IPE32)
 * - Where's Waldo? Exploring Geography
 * - Eraser Turnabout by Imagination Pilots
 * - Virtual K'Nex by Imagination Pilots
 * ART file packer and unpacker by Daniel Marschall, ViaThinkSoft (C) 2018
 * Revision: 2018-02-15
 **/

#ifndef __inc__ipe32_lzw_decoder
#define __inc__ipe32_lzw_decoder

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct tagIpe32LZWDecoder {
	unsigned int *prefix_code;            /* This array holds the prefix codes */
	unsigned char *append_character;      /* This array holds the appended chars */
	unsigned char decode_stack[4000];     /* This array holds the decoded string */

	int num_bits;                         /* Starting with 9 bit codes */
	int max_code;                         /* old MAX_CODE */

	int input_bit_count;
	uint32_t input_bit_buffer;
} Ipe32LZWDecoder;

// Returns: Bytes written or -1 when an error occurs
int ipe32lzw_decode(Ipe32LZWDecoder *decoder, unsigned char* outputBuffer, const size_t outpufBufferSize, unsigned char* lzwInputBuffer, const size_t maxReadBytes);

Ipe32LZWDecoder* new_ipe32lzw_decoder(void);
void ipe32lzw_init_decoder(Ipe32LZWDecoder *decoder);
void ipe32lzw_free_decoder(Ipe32LZWDecoder *decoder);

#endif // #ifndef __inc__ipe32_lzw_decoder

