
#ifndef KE_ASM_UTILS_H
#define KE_ASM_UTILS_H

#include "sys/types.h"

void store_gdt();
void load_gdt();
void load_idt();
void load_tss();

void clear_intr();
void set_intr();
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
void putax(char); // display char in top corner

// exception handlers
void	divide_error();
void	single_step_exception();
void	nmi();
void	breakpoint_exception();
void	overflow();
void	bounds_check();
void	inval_opcode();
void	copr_not_available();
void	double_fault();
void	copr_seg_overrun();
void	inval_tss();
void	segment_not_present();
void	stack_exception();
void	general_protection();
void	page_fault();
void	copr_error();

// interrupt handlers
void	irq00(); 
void	irq01();
void	irq02();
void	irq03();
void	irq04();
void	irq05();
void	irq06();
void	irq07();
void	irq08();
void	irq09();
void	irq10();
void	irq11();
void	irq12();
void	irq13();
void	irq14();
void	irq15();

void restart();

#endif