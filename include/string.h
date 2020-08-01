#ifndef MINOS_STRING_H
#define MINOS_STRING_H
#include "types.h"

void* memset(void *dst, int val, size_t count);
void* memcpy(void* dst, const void* src, size_t size);
int memcmp(const void* s1, const void* s2, size_t size);

void strcpy(char* dst, const char* src);
int strcmp(const char * s1, const char *s2);
char * strcat(char * s1, const char *s2);
size_t strlen(const char* str);
void reverse_str(char str[], int length);
uint32_t digit_count(uint32_t num);

#endif