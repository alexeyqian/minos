#ifndef _STRING_H
#define _STRING_H

#include "sys/types.h"

void* memset(void *dst, int val, size_t count);
void* memcpy(void* dst, const void* src, size_t size);
int memcmp(const void* s1, const void* s2, size_t size);

void strcpy(char* dst, const char* src);
int strcmp(const char * s1, const char *s2);
char * strcat(char * s1, const char *s2);
size_t strlen(const char* str);
void reverse_str(char str[], int length);
int digit_count(int num);

#endif