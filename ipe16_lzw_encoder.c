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

#include "ipe16_lzw_encoder.h"
#include "utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

Ipe16LZWEncoder* new_ipe16lzw_encoder(void) {
	return (Ipe16LZWEncoder*)app_zero_alloc(sizeof(Ipe16LZWEncoder));
}

void del_ipe16lzw_encoder(Ipe16LZWEncoder* encoder) {
	free(encoder);
}

void ipe16lzw_write_code(FILE* outFile, Ipe16LZWEncoder* encoder, int code) {
	if (code == FLUSH_OUTPUT) {
		/* write all remaining data */
		while (encoder->shift_state > 0) {
			fputc(encoder->shift_data & 0xff, outFile);
			encoder->shift_data >>= 8;
			encoder->shift_state -= 8;
		}
		encoder->shift_state = 0;
	} else {
		encoder->shift_data |= ((long) code) << encoder->shift_state;
		encoder->shift_state += encoder->running_bits;

		while (encoder->shift_state >= 8) {
			/* write full bytes */
			fputc(encoder->shift_data & 0xff, outFile);
			encoder->shift_data >>= 8;
			encoder->shift_state -= 8;
		}
	}

	if (encoder->running_code >= encoder->max_code_plus_one && code <= LZ_MAX_CODE) {
		encoder->max_code_plus_one = 1 << ++encoder->running_bits;
	}
}

static void ipe16lzw_clear_hash_table(unsigned long* hash_table) {
	int i;
	for (i=0; i<HT_SIZE; i++)  {
		hash_table[i] = 0xFFFFFFFFL;
	}
}

void ipe16lzw_init_encoder(Ipe16LZWEncoder* encoder) {
	encoder->running_code = FIRST_CODE;
	encoder->running_bits = LZ_MIN_BITS;
	encoder->max_code_plus_one = 1 << encoder->running_bits;
	encoder->shift_state  = 0;
	encoder->shift_data   = 0;
}

static int ipe16lzw_hash_key(unsigned long key) {
	return ((key >> 12) ^ key) & HT_KEY_MASK;
}

static int ipe16lzw_lookup_hash(unsigned long* hash_table, unsigned long key) {
	int hkey = ipe16lzw_hash_key(key);
	unsigned long htkey;

	while ((htkey = HT_GET_KEY(hash_table[hkey])) != 0xFFFFFL) {
		if (key == htkey) {
			return HT_GET_CODE(hash_table[hkey]);
		}
		hkey = (hkey + 1) & HT_KEY_MASK;
	}

	return -1;
}

static void ipe16lzw_add_hash_entry(unsigned long* hash_table, unsigned long key, int code) {
	int hkey = ipe16lzw_hash_key(key);

	while (HT_GET_KEY(hash_table[hkey]) != 0xFFFFFL) {
		hkey = (hkey + 1) & HT_KEY_MASK;
	}
	hash_table[hkey] = HT_PUT_KEY(key) | HT_PUT_CODE(code);
}

void ipe16lzw_encode(FILE* outFile, Ipe16LZWEncoder* encoder, unsigned char* input, int inputLength) {
	int i = 0, current_code, new_code;
	unsigned long new_key;
	unsigned char pixval;
	
	/* Init stuff */
	ipe16lzw_init_encoder(encoder);
	ipe16lzw_clear_hash_table(encoder->hash_table);
	ipe16lzw_write_code(outFile, encoder, CLEAR_CODE);	

	if (inputLength == 0) return;
	current_code = input[i++];

	while (i < inputLength) {
		pixval = input[i++]; /* Fetch next pixel from stream */

		new_key = (((unsigned long) current_code) << 8) + pixval;
		if ((new_code = ipe16lzw_lookup_hash(encoder->hash_table, new_key)) >= 0) {
			current_code = new_code;
		} else {
			ipe16lzw_write_code(outFile, encoder, current_code);
			current_code = pixval;

			if (encoder->running_code >= LZ_MAX_CODE) {
				ipe16lzw_write_code(outFile, encoder, CLEAR_CODE);
				encoder->running_code = FIRST_CODE;
				encoder->running_bits = LZ_MIN_BITS;
				encoder->max_code_plus_one = 1 << encoder->running_bits;
				ipe16lzw_clear_hash_table(encoder->hash_table);
			} else {
				/* Put this unique key with its relative code in hash table */
				ipe16lzw_add_hash_entry(encoder->hash_table, new_key, encoder->running_code++);
			}
		}
	}

	/* Flush */
	ipe16lzw_write_code(outFile, encoder, current_code);
	ipe16lzw_write_code(outFile, encoder, END_CODE);
	ipe16lzw_write_code(outFile, encoder, FLUSH_OUTPUT);
}

