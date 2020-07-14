#include "klib.h"
#include "const.h"
#include "types.h"
#include "ke_asm_utils.h"

char* memset(char* buf, char value, int size){
	int i;
    for(i = 0; i < size; i++)
        buf[i] = value;
        
    return buf;
}
/*
int memcmp(const char* a, const char* b, int size) {
	int i;	
	for (i = 0; i < size; i++) {
		if (a[i] < b[i])
			return -1;
		else if (b[i] < a[i])
			return 1;
	}
	return 0;
}*/

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
/*
uint32_t digit_count(int num){
    uint32_t count = 0;
    if(num == 0) return 1;
    while(num > 0){
        num /= 10;
        count ++;
    }

    return count;
}*/

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

char* itox( int num, char* str) {
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

void delay(int time){
	int i, j, k;
	for(i = 0; i < time; i++)
		for(j = 0; j < 1000; j++)
			for(k = 0; k < 1000; k ++){}
}

void milli_delay(int milli_sec){
    int t = get_ticks();
    while(((get_ticks() - t) * 1000 / HZ) < milli_sec) {}
}

// TODO: merge to itoa
// leading 0s are ignored, ex. 0000B800 displayed as B800 
char * itoa2(char * str, int num)
{
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


int vsprintf(char* buf, const char* fmt, va_list args){
    char* p;
    char tmp[256];
    va_list p_next_arg = args;

    for(p = buf; *fmt; fmt++){
        if(*fmt != '%'){
            *p++ = *fmt;
            continue;
        }

        fmt++;

        switch(*fmt){
            case 'x':
                itoa2(tmp, *((int*)p_next_arg));
                strcpy(p, tmp);
                p_next_arg += 4;
                p += strlen(tmp);
                break;
            case 's':
                break;
            default:
                break;
        }
    }

    return (p-buf);
}

/******************************************************************************************
                                     Example
===========================================================================================

i = 0x23;
j = 0x78;
char fmt[] = "%x%d";
printf(fmt, i, j);

        push    j
        push    i
        push    fmt
        call    printf
        add     esp, 3 * 4


                ┃   HIGH   ┃                        ┃   HIGH   ┃
                ┃   ...    ┃                        ┃   ...    ┃
                ┣━━━━━━━━━━┫                        ┣━━━━━━━━━━┫
                ┃          ┃                 0x32010┃   '\0'   ┃
                ┣━━━━━━━━━━┫                        ┣━━━━━━━━━━┫
         0x3046C┃   0x78   ┃                 0x3200c┃    d     ┃
                ┣━━━━━━━━━━┫                        ┣━━━━━━━━━━┫
   arg = 0x30468┃   0x23   ┃                 0x32008┃    %     ┃
                ┣━━━━━━━━━━┫                        ┣━━━━━━━━━━┫
         0x30464┃ 0x32000  ╂───--------─┐    0x32004┃    x     ┃
                ┣━━━━━━━━━━┫            │           ┣━━━━━━━━━━┫
                ┃          ┃            └──→ 0x32000┃    %     ┃
                ┣━━━━━━━━━━┫                        ┣━━━━━━━━━━┫
                ┃    ...   ┃                        ┃   ...    ┃
                ┃    LOW   ┃                        ┃   LOW    ┃

    here is how vsprintf get called.
    vsprintf(buf, 0x32000, 0x30468);
*/

// C calling convension is caller clear the params in stack
// since for this type of variable params,
// only caller knows how many params used.
int printf(const char *fmt, ...){
    int i;
    char buf[256];
    va_list args = (va_list)((char*)(&fmt) + 4); // points to next params after fmt
    // now args is actually the addr of arg1 just behind fmt
    // args is actually a char*
    i = vsprintf(buf, fmt, args); 
    write(buf, i);
    return i;
}