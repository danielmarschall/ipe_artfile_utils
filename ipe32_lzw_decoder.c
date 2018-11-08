/**
 * LZW Decoder for Imagination Pilots Entertainment 32-bit games (IPE32)
 * - Where's Waldo? Exploring Geography
 * - Eraser Turnabout by Imagination Pilots
 * - Virtual K'Nex by Imagination Pilots
 * ART file packer and unpacker by Daniel Marschall, ViaThinkSoft (C) 2018
 * Revision: 2018-02-15
 *
 * Based on  : Basic LZW Data Compression program published in DDJ October 1989 issue.
 *             by Mark R. Nelson
 *             http://collaboration.cmc.ec.gc.ca/science/rpn/biblio/ddj/Website/articles/DDJ/1989/8910/8910b/8910b.htm
 * Updated by: Shawn M. Regan, January 1990
 *             http://mirror.bagelwood.com/textfiles/computers/regan.lst
 * Updated by: Daniel Marschall, 11 February 2018
 *             https://misc.daniel-marschall.de/code/c/lzw.c
 * Changed for IPE32: - Simplified
 *                    - Thread safe
 *                    - MAX_BITS = 13
 **/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "utils.h"
#include "ipe32_lzw_decoder.h"

#define INIT_BITS 9
#define MAX_BITS 13           /* Do not exceed 14 with this program */

#if MAX_BITS == 14            /* Set the table size. Must be a prime    */
  #define TABLE_SIZE 18041    /* number somewhat larger than 2^MAX_BITS.*/
#elif MAX_BITS == 13
  #define TABLE_SIZE 9029
#else
  #define TABLE_SIZE 5021
#endif

#define CLEAR_TABLE 256    /* Code to flush the string table */
#define TERMINATOR  257    /* To mark EOF Condition, instead of MAX_VALUE */
#define FIRST_CODE  258    /* First available code for code_value table */

#define MAXVAL(n) (( 1 <<( n )) -1)   /* max_value formula macro */

unsigned char* decode_string(Ipe32LZWDecoder *decoder, unsigned char *buffer, unsigned int code) {
	int i=0;

	while (code > 255) {
		*buffer++ = decoder->append_character[code];
		code = decoder->prefix_code[code];
		if (i++ >= 4000) {
			// printf("Error during code expansion\n");
			return NULL;
		}
	}
	*buffer=code;
	return(buffer);
}

unsigned input_code(Ipe32LZWDecoder *decoder, unsigned char* lzwInputBuffer, int* inputBufferPos) {
	unsigned int return_value;

	while (decoder->input_bit_count <= 24) {
		decoder->input_bit_buffer |= lzwInputBuffer[(*inputBufferPos)] << (24 - decoder->input_bit_count);
		(*inputBufferPos)++;
		decoder->input_bit_count += 8;
	}
	return_value=decoder->input_bit_buffer >> (32-decoder->num_bits);
	decoder->input_bit_buffer <<= decoder->num_bits;
	decoder->input_bit_count -= decoder->num_bits;
	return(return_value);
}

void reset_input_buffer(Ipe32LZWDecoder *decoder) {
	decoder->input_bit_count = 0;
	decoder->input_bit_buffer = 0;
}

void ipe32lzw_reset_decoder(Ipe32LZWDecoder *decoder) {
	decoder->num_bits = INIT_BITS;
	decoder->max_code = MAXVAL(decoder->num_bits);         /* Initialize max_value & max_code */

	// Make sure the input buffer is correctly flushed
	reset_input_buffer(decoder);
}

void ipe32lzw_init_decoder(Ipe32LZWDecoder *decoder) {
	decoder->prefix_code      = malloc(TABLE_SIZE*sizeof(unsigned int));
	decoder->append_character = malloc(TABLE_SIZE*sizeof(unsigned char));
	ipe32lzw_reset_decoder(decoder);
}

// Returns: Bytes written or -1 when an error occurs
int ipe32lzw_decode(Ipe32LZWDecoder *decoder, unsigned char* outputBuffer, const size_t outpufBufferSize, unsigned char* lzwInputBuffer, const size_t maxReadBytes) {
	unsigned int next_code=FIRST_CODE;
	unsigned int new_code;
	unsigned int old_code;
	int character;
	int clear_flag=1;          /* Need to clear the code value array */
	unsigned char *string;

	int inputBufferPos = 0;
	int outputBufferPos = 0;
	#define OUTPUT(code) { if (outputBufferPos == outpufBufferSize) return -1; outputBuffer[outputBufferPos++] = code; }

	ipe32lzw_reset_decoder(decoder);

	while (1) {
		if (inputBufferPos == maxReadBytes) return -1;
		if ((new_code=input_code(decoder, lzwInputBuffer, &inputBufferPos)) == TERMINATOR) break;

		if (clear_flag) {            /* Initialize or Re-Initialize */
			clear_flag=0;
			old_code=new_code;   /* The next three lines have been moved */
			character=old_code;  /* from the original */
			OUTPUT(old_code);
			continue;
		}
		if (new_code == CLEAR_TABLE) {     /* Clear string table */
			clear_flag=1;
			decoder->num_bits=INIT_BITS;
			next_code=FIRST_CODE;
			decoder->max_code = MAXVAL(decoder->num_bits);
			continue;
		}
		if (new_code >= next_code) {       /* Check for string+char+string */
			*decoder->decode_stack=character;
			string = decode_string(decoder, decoder->decode_stack+1,old_code);
		} else {
			string = decode_string(decoder, decoder->decode_stack,new_code);
		}
		if (string == NULL) return -1;

		character = *string;              /* Output decoded string in reverse */
		while (string >= decoder->decode_stack) {
			OUTPUT(*string--);
		}

		if (next_code <= decoder->max_code) {      /* Add to string table if not full */
			decoder->prefix_code[next_code]=old_code;
			decoder->append_character[next_code++]=character;
			if (next_code == decoder->max_code && decoder->num_bits < MAX_BITS) {
				decoder->max_code = MAXVAL(++decoder->num_bits);
			}
		}
		old_code=new_code;
	}

	return outputBufferPos;
}

void ipe32lzw_free_decoder(Ipe32LZWDecoder *decoder) {
	free(decoder->prefix_code);
	free(decoder->append_character);
}

Ipe32LZWDecoder* new_ipe32lzw_decoder(void) {
	return (Ipe32LZWDecoder*)app_zero_alloc(sizeof(Ipe32LZWDecoder));
}
