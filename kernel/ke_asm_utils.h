
#ifndef MINOS_KE_ASM_UTILS_H
#define MINOS_KE_ASM_UTILS_H
#include "types.h"

//struct proc;
struct s_message;

// system calls, user level
void syscall();             
int  get_ticks();    
void write(char* buf, int len);  
int	printx(char* str);
int sendrec(int function, int src_dest, struct s_message* m); 

// util functions called by c language vmmgr
void disable_int();
void enable_int();

void store_gdt();
void load_gdt();
void load_idt();
void load_tss();

void enable_irq(int irq);
void disable_irq(int irq);

uint32_t get_cr0();
void load_pdbr (uint32_t addr); // load cr3
uint32_t get_pdbr();            // get  cr3
void flush_tlb_entry(uint32_t addr);
void enable_paging();
void disable_paging();

void out_byte(uint16_t port, uint8_t value);
uint8_t in_byte(uint16_t port);

void halt();

#endif