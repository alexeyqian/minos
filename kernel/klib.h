#ifndef MINOS_KLIB_H
#define MINOS_KLIB_H

#include "types.h"
#include "ktypes.h"
#include "global.h"

#define	STR_DEFAULT_LEN	1024

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
int sprintf(char* buf, const char* fmt, ...);

void assertion_failure(char* exp, char* file, char* base_file, int line);
#define assert(exp) if(exp);else assertion_failure(#exp, __FILE__, __BASE_FILE__, __LINE__)
void panic(const char *fmt, ...);

#define proc2pid(x) (x - proc_table)
void* va2la(int pid, void*va);

void put_irq_handler(int irq, pf_irq_handler_t handler);

#define printl printf


/**
 * `phys_copy' and `phys_set' are used only in the kernel, where segments
 * are all flat (based on 0). In the meanwhile, currently linear address
 * space is mapped to the identical physical address space. Therefore,
 * a `physical copy' will be as same as a common copy, so does `phys_set'.
 */
#define	phys_copy	memcpy
#define	phys_set	memset

void reset_msg(MESSAGE* p);

#endif