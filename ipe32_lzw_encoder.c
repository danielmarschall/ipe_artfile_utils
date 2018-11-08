/**
 * LZW Encoder for Imagination Pilots Entertainment 32-bit games (IPE32)
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
#include "ipe32_lzw_encoder.h"

#define INIT_BITS 9
#define MAX_BITS 13           /* Do not exceed 14 with this program */
#define HASHING_SHIFT (MAX_BITS - 8)

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
#define CHECK_TIME  100    /* Check comp ratio every CHECK_TIME chars input */

#define MAXVAL(n) (( 1 <<( n )) -1)   /* max_value formula macro */

unsigned int find_match(Ipe32LZWEncoder *encoder, int hash_prefix, unsigned int hash_character) {
	int index, offset;

	index = (hash_character << HASHING_SHIFT) ^ hash_prefix;

	offset = (index == 0) ? 1 : TABLE_SIZE - index;

	while (1) {
		if (encoder->code_value[index] == -1) {
			return(index);
		}
		if (encoder->prefix_code[index] == hash_prefix && encoder->append_character[index] == hash_character) {
			return(index);
		}
		index -= offset;
		if (index < 0) {
			index += TABLE_SIZE;
		}
	}
}

void output_code(Ipe32LZWEncoder *encoder, unsigned int code, unsigned char* outBuf, size_t* compressedPos) {
	encoder->output_bit_buffer |= (uint32_t) code << (32 - encoder->num_bits - encoder->output_bit_count);
	encoder->output_bit_count += encoder->num_bits;
	while (encoder->output_bit_count >= 8) {
		outBuf[(*compressedPos)] = encoder->output_bit_buffer >> 24; // putc(output_bit_buffer >> 24, output);
		(*compressedPos)++;

		encoder->output_bit_buffer <<= 8;
		encoder->output_bit_count -= 8;
		encoder->bytes_out++;
	}
}

void reset_output_buffer(Ipe32LZWEncoder *encoder) {
	encoder->output_bit_count = 0;
	encoder->output_bit_buffer = 0;
}

void ipe32lzw_reset_encoder(Ipe32LZWEncoder *encoder) {
	encoder->num_bits = INIT_BITS;
	encoder->max_code = MAXVAL(encoder->num_bits);         /* Initialize max_value & max_code */
	encoder->bytes_in = 0;
	encoder->bytes_out = 0;
	encoder->checkpoint = CHECK_TIME;

	// For some reason, the output buffer doesn't get correctly flushed when
	// a new compression is started. So I made the static symbols global
	// and clear them here.
	reset_output_buffer(encoder);
}

// Returns: Bytes written, or -1 if compression failed
int ipe32lzw_encode(Ipe32LZWEncoder *encoder, unsigned char* compressedData, const size_t compressedBufLen, unsigned char* uncompressedData, const size_t uncompressedSize) {
	unsigned int next_code=FIRST_CODE;
	unsigned int index;
	int i,                 /* All purpose integer */
	    ratio_new,         /* New compression ratio as a percentage */
	    ratio_old=100;     /* Original ratio at 100% */

	ipe32lzw_reset_encoder(encoder);

	for (i=0; i<TABLE_SIZE; ++i) {   /* Initialize the string table first */
		encoder->code_value[i]=-1;
	}

	/* Get the first code */
	if (compressedBufLen == 0) return -1;
	unsigned int string_code = uncompressedData[0]; //string_code=getc(input);
	size_t uncompressedPos = 1;
	size_t compressedPos = 0;
	#define OUTPUT(code) { if (compressedPos == compressedBufLen) return -1; output_code(encoder,code,compressedData,&compressedPos); }

	/* This is the main compression loop. Notice when the table is full we try
	 * to increment the code size. Only when num_bits == MAX_BITS and the code
	 * value table is full do we start to monitor the compression ratio.
	 */
	while (uncompressedPos < uncompressedSize) { // while((character=getc(input)) != (unsigned)EOF) {
		unsigned int character = uncompressedData[uncompressedPos++];

		++encoder->bytes_in;
		index=find_match(encoder,string_code,character);
		if (encoder->code_value[index] != -1) {
			string_code=encoder->code_value[index];
		} else {
			if (next_code <= encoder->max_code) {
				encoder->code_value[index]=next_code++;
				encoder->prefix_code[index]=string_code;
				encoder->append_character[index]=character;
			}
			OUTPUT(string_code);   /* Send out current code */
			string_code=character;
			if (next_code > encoder->max_code) {      /* Is table Full? */
				if (encoder->num_bits < MAX_BITS) {     /* Any more bits? */
					encoder->max_code = MAXVAL(++encoder->num_bits);  /* Increment code size then */
				} else if (encoder->bytes_in > encoder->checkpoint) {         /* At checkpoint? */
					if (encoder->num_bits == MAX_BITS) {
						ratio_new = encoder->bytes_out*100/encoder->bytes_in; /* New compression ratio */
						if (ratio_new > ratio_old) {        /* Has ratio degraded? */
							OUTPUT(CLEAR_TABLE); /* YES,flush string table */
							encoder->num_bits=INIT_BITS;
							next_code=FIRST_CODE;        /* Reset to FIRST_CODE */
							encoder->max_code = MAXVAL(encoder->num_bits); /* Re-Initialize this stuff */
							encoder->bytes_in = encoder->bytes_out = 0;
							ratio_old = 100;             /* Reset compression ratio */
							for (i=0; i<TABLE_SIZE; ++i) {  /* Reset code value array */
								encoder->code_value[i]=-1;
							}
						} else {                                /* NO, then save new */
							ratio_old = ratio_new;          /* compression ratio */
						}
					}
					encoder->checkpoint = encoder->bytes_in + CHECK_TIME;  /* Set new checkpoint */
				}
			}
		}
	}
	OUTPUT(string_code);   /* Output the last code */
	if (next_code == encoder->max_code) {       /* Handles special case for bit */
		++encoder->num_bits;                /* increment on EOF */
	}
	OUTPUT(TERMINATOR);    /* Output the end of buffer code */
	OUTPUT(0);             /* Flush the output buffer */
	OUTPUT(0);
	OUTPUT(0);

	return compressedPos;
}

void ipe32lzw_init_encoder(Ipe32LZWEncoder *encoder) {
	/* The three buffers for the compression phase. */
	encoder->code_value=malloc(TABLE_SIZE*sizeof(unsigned int));
	encoder->prefix_code=malloc(TABLE_SIZE*sizeof(unsigned int));
	encoder->append_character=malloc(TABLE_SIZE*sizeof(unsigned char));
	ipe32lzw_reset_encoder(encoder);
}

void ipe32lzw_free_encoder(Ipe32LZWEncoder *encoder) {
	free(encoder->code_value);                    /* Needed only for compression */
	free(encoder->prefix_code);
	free(encoder->append_character);
}

Ipe32LZWEncoder* new_ipe32lzw_encoder(void) {
	return (Ipe32LZWEncoder*)app_zero_alloc(sizeof(Ipe32LZWEncoder));
}
