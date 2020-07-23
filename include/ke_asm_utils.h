
#ifndef MINOS_KE_ASM_UTILS_H
#define MINOS_KE_ASM_UTILS_H
#include "types.h"

void store_gdt();
void load_gdt();
void load_idt();
void load_tss();

void disable_int();
void enable_int();
void enable_irq(int irq);
void disable_irq(int irq);
void out_byte(uint16_t port, uint8_t value);
uint8_t in_byte(uint16_t port);
void port_read(uint16_t port, void* buf, int n);
void port_write(uint16_t port, void* buf, int n);

uint32_t get_cr0();
void load_pdbr (uint32_t addr); // load cr3
uint32_t get_pdbr();            // get  cr3
void flush_tlb_entry(uint32_t addr);
void enable_paging();
void disable_paging();

void halt();

struct s_message;
// syscalls
void syscall();             
int  get_ticks();    
void write(char* buf, int len);  
int	printx(char* str);
int sendrec(int function, int src_dest, struct s_message* m); 

#endif