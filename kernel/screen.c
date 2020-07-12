#include "screen.h"

#define VIDEO_ADDRESS 0xb8000
#define MAX_ROWS 25
#define MAX_COLS 80
#define WHITE_ON_BLACK 0x0f
// Screen device I/O ports
#define REG_SCREEN_CTRL 0x3D4
#define REG_SCREEN_DATA 0X3D5

extern uint8_t in_byte(io_port_t port);
extern void out_byte(io_port_t port, uint8_t data);
extern void memcpy(char* dst, const char* src, int size);

PRIVATE int _get_cursor(){
    out_byte(REG_SCREEN_CTRL, 14);
    int offset = in_byte(REG_SCREEN_DATA) << 8;
    out_byte(REG_SCREEN_CTRL, 15);
    offset += in_byte(REG_SCREEN_DATA);
    return offset * 2;
}

PRIVATE void _set_cursor(int offset){
    offset /= 2;
    out_byte(REG_SCREEN_CTRL, 14);
    out_byte(REG_SCREEN_DATA, (unsigned char)(offset >> 8));
    out_byte(REG_SCREEN_CTRL, 15);
    out_byte(REG_SCREEN_DATA, (unsigned char)(offset));
}

PRIVATE int _get_screen_offset(int row, int col){
    return (row * MAX_COLS + col) * 2;
}

PRIVATE int _scroll(int cursor_offset){
    // if the cursor is within the scree, return it unmodified
    if(cursor_offset < MAX_ROWS * MAX_COLS * 2)
        return cursor_offset;

    // shuffle the rows back one.
    int i;
    for(i = 1; i < MAX_ROWS; i++)
        memcpy(
            (char*)(_get_screen_offset(i-1, 0) + VIDEO_ADDRESS),
			(char*)(_get_screen_offset(i, 0)   + VIDEO_ADDRESS),            
            MAX_COLS*2);

    // Black the last line by setting all bytes to 0
    char* last_line = (char*)(_get_screen_offset(MAX_ROWS - 1, 0) + VIDEO_ADDRESS);
    for(i = 0; i < MAX_COLS*2; i++)
        last_line[i] = 0;

    cursor_offset -= 2*MAX_COLS;
    return cursor_offset;
}

PUBLIC void clear_screen(){
    for(int row = 0; row < MAX_ROWS; row++)
        for(int col = 0; col < MAX_COLS; col++)
            print_char(' ', row, col, WHITE_ON_BLACK);
    
    _set_cursor(_get_screen_offset(0, 0));
}

PUBLIC void print_char(char c, int row, int col, char attribute){
    unsigned char* vidmem = (unsigned char *) VIDEO_ADDRESS;
    if(!attribute)
        attribute = WHITE_ON_BLACK;

    int offset;
    if(col >= 0 && row >= 0)
        offset = _get_screen_offset(row, col);
    else // use the current cursor position
        offset = _get_cursor();

    if(c == '\n'){
        int rows = offset / (2 * MAX_COLS);
        offset = _get_screen_offset(rows, MAX_COLS - 1);
    }else{
        vidmem[offset] = c;
        vidmem[offset+1] = attribute;
    }

    offset += 2;
    // make scrolling adjustment, for when we reach the bottom of the screen
    offset = _scroll(offset);
    // update the cursor position on the screen device
    _set_cursor(offset);
}
