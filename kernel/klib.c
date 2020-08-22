#include "klib.h"
#include "const.h"
#include "types.h"
#include "ktypes.h"
#include "global.h"
#include "ke_asm_utils.h"
#include "assert.h"
#include "screen.h"
#include "vsprintf.h"

PUBLIC char* itoa(int num, char* str, int base){
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

/*
PUBLIC char* i2a(int val, int base, char ** ps)
{
	int m = val % base;
	int q = val / base;
	if (q) {
		i2a(q, base, ps);
	}
	*(*ps)++ = (m < 10) ? (m + '0') : (m - 10 + 'A');

	return *ps;
}*/

/*
 * <ring 0-1> calculate the linear address of proc's segment
 * @param p: proc pointer
 * @param idx: index of proc's segments
 * 
 * @return base address of memory pointed by LDT descriptor
 * */
PRIVATE int ldt_seg_linear(struct proc* p, int idx){
	struct descriptor* d = &p->ldt[idx];
	return d->base_high << 24 | d->base_mid << 16 | d->base_low;
}

/* 
 * virtual address map to linear address for pid
 * since each proc has it's own LDT, 
 * and each LDT descriptor is representing differrent memory range
 * 
 * @return pointer to linear address
* */
PUBLIC void* va2la(int pid, void* va){
	struct proc* p = &proc_table[pid];
	uint32_t seg_base = ldt_seg_linear(p, INDEX_LDT_RW);
	uint32_t la = seg_base + (uint32_t)va;

	if(pid < NR_TASKS + NR_NATIVE_PROCS)
        kassert(la == (uint32_t)va);
    		
	return (void*)la;
}