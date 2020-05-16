#ifndef _MINOS_KLIB_H_
#define _MINOS_KLIB_H_

#include "types.h"

// IO read and write
// in: read data from device register, out: write data to device register
// I/O addresses/ports that are mapped to speciﬁc controller registers
uint8_t in_byte(io_port_t port);
void out_byte(io_port_t port, uint8_t data);
uint16_t in_word(io_port_t port);
void out_word(io_port_t, uint16_t data);

char* memset(char *dst, char val, int count);
int memcmp(const char*, const char*, int);
void memcpy(char* dst, const char* src, int size);

// ==================  screen.h ================== 
#define VIDEO_ADDRESS 0xb8000
#define MAX_ROWS 25
#define MAX_COLS 80
#define WHITE_ON_BLACK 0x0f
// Screen device I/O ports
#define REG_SCREEN_CTRL 0x3D4
#define REG_SCREEN_DATA 0X3D5

void clear_screen();
int  get_cursor();
void set_cursor(int offset);
void print_at(char* str, int row, int col);
void print_char(char c, int row, int col, char attribute);
// ================== end of screen.h ================== 

uint32_t digit_count(int num);
int strlen(const char* str);
// integer to zero terminated string
char* itoa(int num, char* str, int base);
// integer to hex, prefix 0s are ignored
char* itox(int num, char* str);
void kprint(char* str);
void print_int(int num);
void print_int_as_hex(int num);

#endif