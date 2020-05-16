#ifndef KLIB_H
#define KLIB_H

#include "types.h"
#include "screen.h"

char *memset(char *dest, char val, int count);
void kmemcpy(char* source, char* dest, int num_bytes);
int memcmp(const void*, const void*, size_t);

size_t strlen(const char*);
char* itoa(int num, char* str, int base);

void kprint(char *str);
void kprintf(const char* format, ...);

void abort(void);

#endif