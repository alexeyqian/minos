#include "types.h"

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

// prefix 0s are ignored
char *itoa(char* str, int num) {
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

//void kprint_str(char* str);
void kprint_int_as_hex(int input){
    //char output[16];
    //itoa(output, input);
    //kprint_str(output);
}

