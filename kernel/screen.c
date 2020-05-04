#include "screen.h"
#include "low_level.h"

int get_cursor(){
    port_byte_out(REG_SCREEN_CTRL, 14);
    int offset = port_byte_in(REG_SCREEN_DATA) << 8;
    port_byte_out(REG_SCREEN_CTRL, 15);
    offset += port_byte_in(REG_SCREEN_DATA);
    return offset * 2;
}

void set_cursor(int offset){
    offset /= 2;
    port_byte_out(REG_SCREEN_CTRL, 14);
    port_byte_out(REG_SCREEN_DATA, (unsigned char)(offset >> 8));
    port_byte_out(REG_SCREEN_CTRL, 15);
    port_byte_out(REG_SCREEN_DATA, (unsigned char)(offset));
}

int get_screen_offset(int row, int col){
    return (row * MAX_COLS + col) * 2;
}

void clear_screen(){
    for(int row = 0; row < MAX_ROWS; row++)
        for(int col = 0; col < MAX_COLS; col++)
            print_char(' ', row, col, WHITE_ON_BLACK);
    
    set_cursor(get_screen_offset(0, 0));
}

int scroll(int cursor_offset){
    // if the cursor is within the scree, return it unmodified
    if(cursor_offset < MAX_ROWS * MAX_COLS * 2)
        return cursor_offset;

    // shuffle the rows back one.
    int i = 0;
    for(i = 1; i < MAX_ROWS; i++)
        memory_copy((char*)(get_screen_offset(i, 0)   + VIDEO_ADDRESS),
                    (char*)(get_screen_offset(i-1, 0) + VIDEO_ADDRESS),
                    MAX_COLS*2);

    // Black the last line by setting all bytes to 0
    char* last_line = (char*)(get_screen_offset(MAX_ROWS - 1, 0) + VIDEO_ADDRESS);
    for(i = 0; i < MAX_COLS*2; i++)
        last_line[i] = 0;

    cursor_offset -= 2*MAX_COLS;
    return cursor_offset;
}

void print_char(char c, int row, int col, char attribute){
    unsigned char* vidmem = (unsigned char *) VIDEO_ADDRESS;
    if(!attribute)
        attribute = WHITE_ON_BLACK;

    int offset;
    if(col >= 0 && row >= 0)
        offset = get_screen_offset(row, col);
    else // use the current cursor position
        offset = get_cursor();

    if(c == '\n'){
        int rows = offset / (2 * MAX_COLS);
        offset = get_screen_offset(rows, MAX_COLS - 1);
    }else{
        vidmem[offset] = c;
        vidmem[offset+1] = attribute;
    }

    offset += 2;
    // make scrolling adjustment, for when we reach the bottom of the screen
    offset = scroll(offset);
    // update the cursor position on the screen device
    set_cursor(offset);
}

void kprint(char* str){
    for(int i=0; str[i]!='\0'; i++)
		print_char(str[i], -1, -1, 0);
}

void kprintf(const char* format, ...){
    
}