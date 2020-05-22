
#ifndef _MINOS_ASM_UTIL_H_
#define _MINOS_ASM_UTIL_H_

extern void store_gdt();
extern void load_gdt();
extern void load_idt();

extern void enable_irq(int irq);
extern void disable_irq(int irq);
extern void disable_int();
extern void enable_int();

extern void syscall();             
extern int  get_ticks();          

#endif