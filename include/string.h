#ifndef MINOS_STRING_H
#define MINOS_STRING_H
#include "types.h"

#define	STR_DEFAULT_LEN	1024

void* memset(void *dst, int val, size_t count);
void* memcpy(void* dst, const void* src, size_t size);
int memcmp(const void* s1, const void* s2, size_t size);

void reverse_str(char str[], int length);
void strcpy(char* dst, const char* src);
size_t strlen(const char* str);

uint32_t digit_count(uint32_t num);

#endif