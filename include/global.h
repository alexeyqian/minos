#ifndef MINOS_GLOBAL_H
#define MINOS_GLOBAL_H

#include "types.h"
#include "ktypes.h"
#include "fs.h" 

// TODO: EXTERN can be extern in it's c file, so below ifdef is not necessary
#ifdef GLOBAL_VARIABLES_HERE
#undef EXTERN
#define EXTERN
#endif

EXTERN int                k_reenter;
EXTERN uint32_t           g_disp_pos; 
EXTERN uint32_t           ticks;
EXTERN struct boot_params g_boot_params;

EXTERN uint8_t              gdt_ptr[6];	               
EXTERN struct descriptor    gdt[GDT_SIZE];
EXTERN uint8_t			    idt_ptr[6];	               
EXTERN struct gate			idt[IDT_SIZE];
EXTERN struct tss           tss;                            // only one tss in kernel, resides in mem, and it has a descriptor in gdt.

EXTERN struct proc*        p_proc_ready;                    // points to next about to run process's pcb in proc_table
EXTERN struct proc         proc_table[NR_TASKS + NR_PROCS]; // contains array of process control block: proc

EXTERN pf_irq_handler_t    irq_table[IRQ_NUM];
EXTERN syscall_t           syscall_table[NR_SYSCALLS];

// fs
extern	uint8_t*  fsbuf;
extern	const int FSBUF_SIZE;
// mm
extern	uint8_t*  mmbuf;
extern	const int MMBUF_SIZE;


// tty
EXTERN int key_pressed; // used for clock_handler to wake up TASK_TTY when a key is pressed

#endif