#ifndef KLIB_H
#define KLIB_H

#include "types.h"


void* memset(void*, unsigned char, size_t);
void* memcpy(void*, const void*, size_t);
int memcmp(const void*, const void*, size_t);

size_t strlen(const char*);

//int kputchar(int);
//int kputs(const char*);
//int kprintf(const char*, ...);

void abort(void);

#endif