/**
 * LZW Encoder for Imagination Pilots Entertainment 32-bit games (IPE32)
 * - Where's Waldo? Exploring Geography
 * - Eraser Turnabout by Imagination Pilots
 * - Virtual K'Nex by Imagination Pilots
 * ART file packer and unpacker by Daniel Marschall, ViaThinkSoft (C) 2018
 * Revision: 2018-02-15
 **/

#ifndef __inc__ipe32_lzw_encoder
#define __inc__ipe32_lzw_encoder

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct tagIpe32LZWEncoder {
	int *code_value;                      /* This is the code value array */
	unsigned int *prefix_code;            /* This array holds the prefix codes */
	unsigned char *append_character;      /* This array holds the appended chars */

	int num_bits;                         /* Starting with 9 bit codes */
	uint32_t bytes_in,bytes_out;          /* Used to monitor compression ratio */
	int max_code;                         /* old MAX_CODE */
	uint32_t checkpoint;                  /* For compression ratio monitoring */

	int output_bit_count;
	uint32_t output_bit_buffer;
} Ipe32LZWEncoder;

// Returns: Bytes written, or -1 if compression failed
int ipe32lzw_encode(Ipe32LZWEncoder *encoder, unsigned char* compressedData, const size_t compressedBufLen, unsigned char* uncompressedData, const size_t uncompressedSize);

Ipe32LZWEncoder* new_ipe32lzw_encoder(void);
void ipe32lzw_init_encoder(Ipe32LZWEncoder *encoder);
void ipe32lzw_free_encoder(Ipe32LZWEncoder *encoder);

#endif // #ifndef __inc__ipe32_lzw_encoder
