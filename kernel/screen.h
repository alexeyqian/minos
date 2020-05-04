#ifndef SCREEN_H
#define SCREEN_H

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
void kprint(char *str);
void kprintf(const char* format, ...);
#endif