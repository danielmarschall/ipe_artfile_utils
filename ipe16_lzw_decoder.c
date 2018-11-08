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

#include "ipe16_lzw_decoder.h"
#include "utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

Ipe16LZWDecoder* new_ipe16lzw_decoder(void) {
	return (Ipe16LZWDecoder*)app_zero_alloc(sizeof(Ipe16LZWDecoder));
}

void del_ipe16lzw_decoder(Ipe16LZWDecoder* decoder) {
	free(decoder);
}

void ipe16lzw_init_decoder(Ipe16LZWDecoder* decoder) {
	decoder->running_code = FIRST_CODE;
	decoder->running_bits = LZ_MIN_BITS;
	decoder->max_code_plus_one = 1 << decoder->running_bits;
	decoder->shift_state  = 0;
	decoder->shift_data   = 0;

	int i;
	for (i = 0; i <= LZ_MAX_CODE; i++) {
		decoder->prefix[i] = NO_SUCH_CODE;
	}
}

int ipe16lzw_read_code(FILE* inFile, Ipe16LZWDecoder* decoder) {
	int code;
	unsigned char next_byte;
	static int code_masks[] = {
		0x0000, 0x0001, 0x0003, 0x0007,
		0x000f, 0x001f, 0x003f, 0x007f,
		0x00ff, 0x01ff, 0x03ff, 0x07ff,
		0x0fff
	};

	while (decoder->shift_state < decoder->running_bits) {
		next_byte = read_byte(inFile);
		decoder->shift_data |=
		  ((unsigned long) next_byte) << decoder->shift_state;
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

// We don't do unsigned, because we want to have <0 as error result
/*unsigned*/ int ipe16lzw_decode(FILE* inFile, Ipe16LZWDecoder* decoder, unsigned char* output, int outputLength) {
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

	prefix		= decoder->prefix;
	suffix		= decoder->suffix;
	stack		= decoder->stack;

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
		} else if (current_code == CLEAR_CODE) {
			for (j = 0; j <= LZ_MAX_CODE; j++) {
				prefix[j] = NO_SUCH_CODE;
			}
			decoder->running_code = FIRST_CODE;
			decoder->running_bits = LZ_MIN_BITS;
			decoder->max_code_plus_one = 1 << decoder->running_bits;
			prev_code = NO_SUCH_CODE;
		} else {
			if (current_code < CLEAR_CODE) {
				output[i++] = current_code;
				//if (i > bytes_written) bytes_written = i;
				++bytes_written;
			} else {
				if ((current_code < 0) || (current_code > LZ_MAX_CODE))
					return -2; /* image defect */
				if (prefix[current_code] == NO_SUCH_CODE) {
					if (current_code == decoder->running_code - 2) {
						current_prefix = prev_code;
						suffix[decoder->running_code - 2]
							= stack[stack_ptr++]
							= ipe16lzw_trace_prefix(prefix, prev_code, CLEAR_CODE);
					} else {
						return -3; /* image defect */
					}
				} else {
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
				   (decoder->running_code > LZ_MAX_CODE+2))
					return -5; /* image defect */
				prefix[decoder->running_code - 2] = prev_code;

				if (current_code == decoder->running_code - 2) {
					suffix[decoder->running_code - 2]
						= ipe16lzw_trace_prefix(prefix, prev_code, CLEAR_CODE);
				} else {
					suffix[decoder->running_code - 2]
						= ipe16lzw_trace_prefix(prefix, current_code, CLEAR_CODE);
				}
			}
			prev_code = current_code;
		}
	}

	return bytes_written;
}
