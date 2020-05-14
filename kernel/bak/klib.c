#include "klib.h"

char* memset(char* bufptr, char value, int size){
    unsigned char* buf = (unsigned char*) bufptr;
    for(size_t i = 0; i < size; i++)
        buf[i] = (unsigned char) value;
        
    return bufptr;
}

void memcpy(char* dstptr, char* srcptr, int size){
    unsigned char* dst = (unsigned char*) dstptr;
    const unsigned char* src = (unsigned char*)srcptr;
    for(size_t i = 0; i < size; i++)
        dst[i] = src[i];    
}

int memcmp(const void* aptr, const void* bptr, size_t size) {
	const unsigned char* a = (const unsigned char*) aptr;
	const unsigned char* b = (const unsigned char*) bptr;
	for (size_t i = 0; i < size; i++) {
		if (a[i] < b[i])
			return -1;
		else if (b[i] < a[i])
			return 1;
	}
	return 0;
}

size_t strlen(const char* str){
    size_t len = 0;
    while(str[len]) len++;
    return len;
}

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

// itoa:  int to string
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
// ltoa: long to string

unsigned int digit_count(int num){
    unsigned int count = 0;
    if(num == 0) return 1;
    while(num > 0){
        num /= 10;
        count ++;
    }

    return count;
}

void kprint(char* str){
    for(int i=0; str[i]!='\0'; i++)
		print_char(str[i], -1, -1, 0);
}

void kprintf(const char* format, ...){
    
}

void print_int(int num){
    char str[digit_count(num) + 1];
    itoa(num, str, 10);
    kprint(str);
}

void abort(void){
    //kprint("kernel: abort()");
    while(1){}
}