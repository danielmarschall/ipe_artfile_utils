#include <stdlib.h>
#include <stdio.h>

#include "utils.h"

size_t file_size(FILE* fp) {
	size_t pos = ftell(fp);
	fseek(fp, 0, SEEK_END);
	size_t len = ftell(fp);
	fseek(fp, pos, SEEK_SET);
	return len;
}

char* sanitize_filename(char* picname) {
	// Remove invalid windows characters
	// TODO: not yet implemented. not necessary for the regular ART files, though
	return picname;
}

void* app_zero_alloc(long bytes) {
	char* ptr = (char*)malloc(bytes);
	int i;
	for (i=0; i<bytes; ++i) ptr[i] = 0;
	return (void*)ptr;
}

unsigned char read_byte(FILE *file) {
	int ch = getc(file);
	if (ch == EOF)
		ch = 0;
	return ch;
}

