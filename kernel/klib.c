#include "klib.h"
#include "const.h"
#include "types.h"
#include "ktypes.h"
#include "global.h"
#include "ke_asm_utils.h"

#include "screen.h"
#include "vsprintf.h"

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
	uint32_t seg_base = (uint32_t)ldt_seg_linear(p, INDEX_LDT_RW);
	uint32_t la = seg_base + (uint32_t)va;

	if(pid < NR_TASKS + NR_NATIVE_PROCS){
		kassert(la == (uint32_t)va);
	}        
    		
	return (void*)la;
}