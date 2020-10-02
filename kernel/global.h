#ifndef GLOBAL_H
#define GLOBAL_H

#include <const.h>
#include <sys/types.h>
#include <minos/const.h>
#include <minos/types.h>

#include "kconst.h"
#include "ktypes.h"

#ifdef GLOBAL_VARIABLES_HERE
#undef EXTERN
#define EXTERN
#endif

EXTERN int                k_reenter;
EXTERN uint32_t           ticks;
EXTERN struct boot_params g_boot_params;

EXTERN uint8_t              gdt_ptr[6];	               
EXTERN struct descriptor    gdt[GDT_SIZE];
EXTERN uint8_t			    idt_ptr[6];	               
EXTERN struct gate			idt[IDT_SIZE];
EXTERN struct tss           tss;                            // only one tss in kernel, resides in mem, and it has a descriptor in gdt.

EXTERN struct proc*        p_proc_ready;                    // points to next about to run process's pcb in proc_table
EXTERN struct proc         proc_table[NR_TASKS + NR_PROCS]; // contains array of process control block: proc

EXTERN pf_irq_handler_t    irq_table[NR_IRQ];
EXTERN syscall_t           syscall_table[NR_SYSCALLS];

#endif