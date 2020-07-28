#ifndef MINOS_GLOBAL_H
#define MINOS_GLOBAL_H

#include "types.h"
#include "ktypes.h"

// TODO: EXTERN can be extern in it's c file, so below ifdef is not necessary
#ifdef GLOBAL_VARIABLES_HERE
#undef EXTERN
#define EXTERN
#endif


// TODO: use get_var() set_var() functions 
// to avoid use global vars in non performance critical areas as much as possible
EXTERN int k_reenter;
EXTERN int ticks;

EXTERN uint8_t              gdt_ptr[6];	               
EXTERN struct descriptor    gdt[GDT_SIZE];
EXTERN uint8_t			    idt_ptr[6];	               
EXTERN struct gate			idt[IDT_SIZE];
EXTERN struct tss           tss;                            // only one tss in kernel, resides in mem, and it has a descriptor in gdt.

EXTERN struct proc*        p_proc_ready;                    // points to next about to run process's pcb in proc_table
EXTERN struct proc         proc_table[NR_TASKS + NR_PROCS]; // contains array of process control block: proc

EXTERN pf_irq_handler_t    irq_table[IRQ_NUM];
EXTERN syscall_t           syscall_table[NR_SYSCALLS];
// TODO: move
EXTERN struct dev_drv_map  dd_map[];

EXTERN uint32_t disp_pos; // for screen.c and tty.c

// fs
EXTERN MESSAGE fs_msg; 
EXTERN struct proc* pcaller;
#endif