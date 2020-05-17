#include "klib.h"

uint8_t in_byte(io_port_t port){
    uint8_t result;
    __asm__("in %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}

void out_byte(io_port_t port , uint8_t data) {
	__asm__("out %%al, %%dx" : :"a" (data) , "d" (port));
}

uint16_t in_word(io_port_t port) {
	uint16_t result ;
	__asm__("in %%dx, %%ax" : "=a" (result) : "d" (port));
	return result ;
}

void out_word(io_port_t port , uint16_t data) {
	__asm__("out %%ax, %%dx " : :"a" (data) , "d" (port));
}

char* memset(char* buf, char value, int size){
	int i;
    for(i = 0; i < size; i++)
        buf[i] = value;
        
    return buf;
}

int memcmp(const char* a, const char* b, int size) {
	int i;	
	for (i = 0; i < size; i++) {
		if (a[i] < b[i])
			return -1;
		else if (b[i] < a[i])
			return 1;
	}
	return 0;
}

void memcpy(char* dst, const char* src, int size){
	int i;
    for(i = 0; i < size; i++)
        dst[i] = src[i];    
}

void strcpy(char* dst, const char* src){
	int i = 0;
    while(src[i] != 0){
        dst[i] = src[i];  
        i++;
    }
}

int get_cursor(){
    out_byte(REG_SCREEN_CTRL, 14);
    int offset = in_byte(REG_SCREEN_DATA) << 8;
    out_byte(REG_SCREEN_CTRL, 15);
    offset += in_byte(REG_SCREEN_DATA);
    return offset * 2;
}

void set_cursor(int offset){
    offset /= 2;
    out_byte(REG_SCREEN_CTRL, 14);
    out_byte(REG_SCREEN_DATA, (unsigned char)(offset >> 8));
    out_byte(REG_SCREEN_CTRL, 15);
    out_byte(REG_SCREEN_DATA, (unsigned char)(offset));
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
// TODO: scroll is not working yet
int scroll(int cursor_offset){
    // if the cursor is within the scree, return it unmodified
    if(cursor_offset < MAX_ROWS * MAX_COLS * 2)
        return cursor_offset;

    // shuffle the rows back one.
    int i = 0;
    for(i = 1; i < MAX_ROWS; i++)
        memcpy(
			(char*)(get_screen_offset(i, 0)   + VIDEO_ADDRESS),
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

uint32_t digit_count(int num){
    uint32_t count = 0;
    if(num == 0) return 1;
    while(num > 0){
        num /= 10;
        count ++;
    }

    return count;
}

int strlen(const char* str){
    int len = 0;
    while(str[len]) len++;
    return len;
}

// reverse string, keep the last '\0'.
void _reverse_str(char str[], int length){
    int start = 0;
    int end = length - 1;
    while(start < end)
    {
        char tmp = str[start];
        str[start++] = str[end];
        str[end--] = tmp;
    }
}

char* itoa(int num, char* str, int base){
    int i = 0;
    int is_negative = 0;

    // special case for num 0
    if(num == 0){
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    // nagative numbers are handeld only with base 10.
    // otherwise numbers are considered unsigned.
    if(num <0 && base == 10){
        is_negative = 1;
        num = -num;
    }

    while(num != 0){
        int rem = num % base;
        str[i++] = (rem > 9)? (rem - 10) + 'a' : rem + '0';
        num = num / base;
    }

    if(is_negative)
        str[i++] = '-';

    // add terminator
    str[i] = '\0';

    _reverse_str(str, i);
    return str;
}

char *itox( int num, char* str) {
    char *	p = str;
	char	ch;
	int	i;
	bool_t	flag = FALSE;

	*p++ = '0';
	*p++ = 'x';

	if(num == 0){
		*p++ = '0';
	}
	else{	
		for(i=28;i>=0;i-=4){
			ch = (num >> i) & 0xF;
			if(flag || (ch > 0)){
				flag = TRUE;
				ch += '0';
				if(ch > '9'){
					ch += 7;
				}
				*p++ = ch;
			}
		}
	}

	*p = 0;

	return str;
}

// TODO: add logic for new line '\n'
void kprint(char* str){
    for(int i=0; str[i]!='\0'; i++)
		print_char(str[i], -1, -1, 0);
}

void print_int(int num){
    //char str[digit_count(num) + 1];
	char str[16];
    itoa(num, str, 10);
    kprint(str);
}

void print_int_as_hex(int num){
    //char str[digit_count(num) + 1];
	char str[16];
    itox(num, str);
    kprint(str);
}

