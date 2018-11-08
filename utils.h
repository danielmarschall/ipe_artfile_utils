#ifndef __inc__utils
#define __inc__utils

#include <stdio.h>

size_t file_size(FILE* fp);
char* sanitize_filename(char* picname);
void* app_zero_alloc(long bytes);
unsigned char read_byte(FILE *file);

#endif // #ifndef __inc__utils

