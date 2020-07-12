#ifndef _MINOS_KLIB_H_
#define _MINOS_KLIB_H_

#include "types.h"
#include "screen.h"

// ==================  low_level.h ================== 
// IO read and write
// in: read data from device register, out: write data to device register
// I/O addresses/ports that are mapped to speciﬁc controller registers
uint8_t in_byte(io_port_t port);
void out_byte(io_port_t port, uint8_t data);

char* memset(char *dst, char val, int count);
int memcmp(const char*, const char*, int);
void memcpy(char* dst, const char* src, int size);
void strcpy(char* dst, const char* src);

void delay(int time);

// ================== end of string.h ================== 
uint32_t digit_count(int num);
int strlen(const char* str);
// integer to zero terminated string
char* itoa(int num, char* str, int base);
// integer to hex, prefix 0s are ignored
char* itox(int num, char* str);
void kprint(char* str);
void print_int(int num);
void print_int_as_hex(int num);

void milli_delay(int milli_sec);
int printf(const char *fmt, ...);

#endif