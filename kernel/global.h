#ifndef _MINOS_GLOBAL_H_
#define _MINOS_GLOBAL_H_

#include "types.h"
#include "ktypes.h"
#include "tty.h"

#ifdef GLOBAL_VARIABLES_HERE
#undef EXTERN
#define EXTERN
#endif

EXTERN int k_reenter;
EXTERN int ticks;

EXTERN uint8_t              gdt_ptr[6];	               
EXTERN struct descriptor    gdt[GDT_SIZE];
EXTERN uint8_t			    idt_ptr[6];	               
EXTERN struct gate			idt[IDT_SIZE];
EXTERN struct tss           tss;                        // only one tss in kernel, resides in mem, and it has a descriptor in gdt.

EXTERN struct proc*        p_proc_ready;               // points to next about to run process's pcb in proc_table
EXTERN struct proc         proc_table[];  // contains array of process control block: proc

// task stack is a mem area divided into MAX_TASK_NUM small areas
// each small area used as stack for a process/task
EXTERN char                task_stack[];

EXTERN struct task         task_table[];
EXTERN struct task         user_proc_table[];	
EXTERN TTY                 tty_table[];
EXTERN syscall_t           syscall_table[];
EXTERN pf_irq_handler_t    irq_table[];

#endif