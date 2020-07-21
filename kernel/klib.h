#ifndef _MINOS_KLIB_H_
#define _MINOS_KLIB_H_

#include "types.h"
#include "ktypes.h"
//#include "screen.h"

char* memset(char *dst, char val, int count);
void memcpy(char* dst, const char* src, int size);
void strcpy(char* dst, const char* src);

void delay_loop(int time);

int strlen(const char* str);
// integer to hex, prefix 0s are ignored
char* itox(int num, char* str);

// below functions are using sys calls inside, cannot be used in kernel
void delay(int milli_sec);
int printf(const char *fmt, ...);

void assertion_failure(char* exp, char* file, char* base_file, int line);
#define assert(exp) if(exp);else assertion_failure(#exp, __FILE__, __BASE_FILE__, __LINE__)
void panic(const char *fmt, ...);

void put_irq_handler(int irq, pf_irq_handler_t handler);

#endif