#ifndef _MINOS_KLIB_H_
#define _MINOS_KLIB_H_

#include "types.h"
#include "screen.h"

char* memset(char *dst, char val, int count);
void memcpy(char* dst, const char* src, int size);
void strcpy(char* dst, const char* src);

void delay_loop(int time);

int strlen(const char* str);
// integer to zero terminated string
char* itoa(int num, char* str, int base);
// integer to hex, prefix 0s are ignored
char* itox(int num, char* str);

// below functions are using sys calls inside, cannot be used in kernel
void delay(int milli_sec);
int printf(const char *fmt, ...);

#endif