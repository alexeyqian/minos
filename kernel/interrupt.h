#ifndef _MINOS_INTERRUPT_H_
#define _MINOS_INTERRUPT_H_

#include "types.h"

// exceptions 
#define	INT_VECTOR_DIVIDE		    0x0
#define	INT_VECTOR_DEBUG		    0x1
#define	INT_VECTOR_NMI			    0x2
#define	INT_VECTOR_BREAKPOINT		0x3
#define	INT_VECTOR_OVERFLOW		    0x4
#define	INT_VECTOR_BOUNDS		    0x5
#define	INT_VECTOR_INVAL_OP		    0x6
#define	INT_VECTOR_COPROC_NOT		0x7
#define	INT_VECTOR_DOUBLE_FAULT		0x8
#define	INT_VECTOR_COPROC_SEG		0x9
#define	INT_VECTOR_INVAL_TSS		0xA
#define	INT_VECTOR_SEG_NOT		    0xB
#define	INT_VECTOR_STACK_FAULT		0xC
#define	INT_VECTOR_PROTECTION		0xD
#define	INT_VECTOR_PAGE_FAULT		0xE
#define	INT_VECTOR_COPROC_ERR		0x10

// 8259A interrupt controller ports
#define	INT_M_CTL	    0x20	/* I/O port for interrupt controller         <Master> */
#define	INT_M_CTLMASK	0x21	/* setting bits in this port disables ints   <Master> */
#define	INT_S_CTL	    0xA0	/* I/O port for second interrupt controller  <Slave>  */
#define	INT_S_CTLMASK	0xA1	/* setting bits in this port disables ints   <Slave>  */

// hardware interrupts
#define IRQ_NUM                     16
#define	CLOCK_IRQ	    0
#define	KEYBOARD_IRQ	1
#define	CASCADE_IRQ   	2	/* cascade enable for 2nd AT controller */
#define	ETHER_IRQ	    3	/* default ethernet interrupt vector */
#define	SECONDARY_IRQ	3	/* RS232 interrupt vector for port 2 */
#define	RS232_IRQ	    4	/* RS232 interrupt vector for port 1 */
#define	XT_WINI_IRQ	    5	/* xt winchester */
#define	FLOPPY_IRQ   	6	/* floppy disk */
#define	PRINTER_IRQ	    7
#define	AT_WINI_IRQ	   14	/* at winchester */

// Interrupt vector IRQ mapping
#define	INT_VECTOR_IRQ0			    0x20
#define	INT_VECTOR_IRQ8			    0x28

#define	INT_VECTOR_SYS_CALL		0x90

// imported functions from kernel_entry.asm
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

void enable_irq(int irq);
void disable_irq(int irq);

// idt
void init_idt_descriptor(unsigned char vector, uint8_t desc_type, pf_int_handler_t handler, unsigned char privilege);
void init_idt();

void put_irq_handler(int irq, pf_irq_handler_t handler);
void irq_handler(int irq);
void clock_handler(int irq);

// system calls
void syscall_handler();     // from syscall.inc
void get_ticks();           // from syscall.inc
int sys_get_ticks_impl();
#endif