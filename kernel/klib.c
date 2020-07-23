#include "klib.h"
#include "const.h"
#include "types.h"
#include "ktypes.h"
#include "global.h"
#include "ke_asm_utils.h"
#include "screen.h"

char* memset(char* buf, char value, int size){
    for(int i = 0; i < size; i++)
        buf[i] = value;
        
    return buf;
}

void memcpy(char* dst, const char* src, int size){
    for(int i = 0; i < size; i++)
        dst[i] = src[i];    
}

void strcpy(char* dst, const char* src){
	int i = 0;
    while(src[i] != 0){
        dst[i] = src[i];  
        i++;
    }
}

uint32_t digit_count(uint32_t num){
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
void reverse_str(char str[], int length){
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

    reverse_str(str, i);
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

PRIVATE char* i2a(int val, int base, char ** ps)
{
	int m = val % base;
	int q = val / base;
	if (q) {
		i2a(q, base, ps);
	}
	*(*ps)++ = (m < 10) ? (m + '0') : (m - 10 + 'A');

	return *ps;
}

PUBLIC int vsprintf_ism(char *buf, const char *fmt, va_list args){
	char* p;
	char tmp[256];
	va_list p_next_arg = args;
	int num;
	for(p=buf; *fmt; fmt++)
		if(*fmt != '%') *p++ = *fmt;
		else{
			fmt++;
			switch(*fmt){
				case 'd':
					num = *((int*)p_next_arg);
					kprint("k1:");					
					kprint_int_as_hex(num);
					break;
				case 'x':
					//memset(q, 0, sizeof(tmp));									
					//i2a(*((int*)p_next_arg), 16, &q);					
					num = *((int*)p_next_arg);
					kprint("k2:");
					kprint_int_as_hex(num);
					itoa(num, tmp, 16);
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

PUBLIC int vsprintf(char *buf, const char *fmt, va_list args)
{
	char*	p;

	va_list	p_next_arg = args;
	int	m;

	char	inner_buf[STR_DEFAULT_LEN];
	char	cs;
	int	align_nr;

	for (p=buf;*fmt;fmt++) {
		if (*fmt != '%') {
			*p++ = *fmt;
			continue;
		}
		else {		/* a format string begins */
			align_nr = 0;
		}

		fmt++;

		if (*fmt == '%') {
			*p++ = *fmt;
			continue;
		}
		else if (*fmt == '0') {
			cs = '0';
			fmt++;
		}
		else {
			cs = ' ';
		}
		while (((unsigned char)(*fmt) >= '0') && ((unsigned char)(*fmt) <= '9')) {
			align_nr *= 10;
			align_nr += *fmt - '0';
			fmt++;
		}

		char * q = inner_buf;
		memset(q, 0, sizeof(inner_buf));

		switch (*fmt) {
		case 'c':
			*q++ = *((char*)p_next_arg);
			p_next_arg += 4;
			break;
		case 'x':
			m = *((int*)p_next_arg);

			i2a(m, 16, &q);
			p_next_arg += 4;
			break;
		case 'd':
			m = *((int*)p_next_arg);
			//kprint("k:");
			//kprint_int_as_hex(m);
			if (m < 0) {
				m = m * (-1);
				*q++ = '-';
			}
			i2a(m, 10, &q);
			
			p_next_arg += 4;
			break;
		case 's':
			strcpy(q, (*((char**)p_next_arg)));
			q += strlen(*((char**)p_next_arg));
			p_next_arg += 4;
			break;
		default:
			break;
		}

		int k;
		for (k = 0; k < ((align_nr > strlen(inner_buf)) ? (align_nr - strlen(inner_buf)) : 0); k++) {
			*p++ = cs;
		}
		q = inner_buf;
		while (*q) {
			*p++ = *q++;
		}
	}

	*p = 0;

	return (p - buf);
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
    buf[i] = 0;
    //write(buf, i);
	printx(buf); // syscall
    return i;
}

// TODO: create printfx as printl/printlx for assert / panic only

int sprintf(char* buf, const char* fmt, ...){
    va_list arg = (va_list)((char *)(&fmt) + 4);
    return vsprintf(buf, fmt, arg);
}

void spin(char* func_name){
    printl(">>> spinning in %s ... \n", func_name);
    while(1){}
}

PUBLIC void assertion_failure(char* exp, char* file, char* base_file, int line){
    printl("%c  assert(%s) failed. file: %s, base_file: %s, ln: %d",
        MAG_CH_ASSERT, exp, file, base_file, line);
    // if assertion fails in a task, the system will halt before printl() returns.
    // if it happens in a user proccess, printl() will return like a common routine.
    
    // use a for ever loop to prevent the proc from going on.
    spin("assertion_failure()");
    // should never arrive here
    //__asm__ __volatile("ud2");
}

PUBLIC void panic(const char *fmt, ...)
{
	int i;
	char buf[256];

	/* 4 is the size of fmt in the stack */
	va_list arg = (va_list)((char*)&fmt + 4);

	i = vsprintf(buf, fmt, arg);

	printl("%c !!panic!! %s", MAG_CH_PANIC, buf);

	/* should never arrive here */
	//__asm__ __volatile__("ud2");
}

// ========================== below functions need global variables =========================

PUBLIC void put_irq_handler(int irq, pf_irq_handler_t handler){
	disable_irq(irq);
	irq_table[irq] = handler;
}

// ring 0-1, calculate the linear address of proc's segment
// idx: index of proc's segments
PRIVATE int ldt_seg_linear(struct proc* p, int idx){
	struct descriptor* d = &p->ldt[idx];
	return d->base_high << 24 | d->base_mid << 16 | d->base_low;
}

// ring 0-1, virtual addr -> linear addr
PUBLIC void* va2la(int pid, void* va){
	struct proc* p = &proc_table[pid];
	uint32_t seg_base = ldt_seg_linear(p, INDEX_LDT_RW);
	uint32_t la = seg_base + (uint32_t)va;

	if(pid < NR_TASKS + NR_PROCS)
		assert(la == (uint32_t)va);
	return (void*)la;
}

PUBLIC void reset_msg(MESSAGE* p)
{
	memset((char*)p, 0, sizeof(MESSAGE));
}