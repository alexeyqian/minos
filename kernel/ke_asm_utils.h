
#ifndef _MINOS_KE_ASM_UTILS_H_
#define _MINOS_KE_ASM_UTILS_H_
#include "types.h"

// system calls
extern void syscall();             
extern int  get_ticks();    
extern void write(char* buf, int len);   

// util functions called by c language vmmgr
extern void disable_int();
extern void enable_int();

extern void store_gdt();
extern void load_gdt();
extern void load_idt();
extern void load_tss();

extern void enable_irq(int irq);
extern void disable_irq(int irq);

extern uint32_t get_cr0();
extern void load_pdbr (uint32_t addr); // load cr3
extern uint32_t get_pdbr();            // get  cr3
extern void flush_tlb_entry(uint32_t addr);
extern void enable_paging();
extern void disable_paging();

extern void out_byte(uint16_t port, uint8_t value);
extern uint8_t in_byte(uint16_t port);

#endif