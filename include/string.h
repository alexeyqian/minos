#ifndef MINOS_STRING_H
#define MINOS_STRING_H
#include "types.h"

#define	STR_DEFAULT_LEN	1024


char* memset(char *dst, char val, int count);
void memcpy(char* dst, const char* src, int size);
int memcmp(const void * s1, const void *s2, int n);

void reverse_str(char str[], int length);
void strcpy(char* dst, const char* src);
int strlen(const char* str);

uint32_t digit_count(uint32_t num);

#endif