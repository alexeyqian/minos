#ifndef _MINOS_GLOBAL_VARS_H
#define _MINOS_GLOBAL_VARS_H

#include "types.h"
#include "const.h"
#include "ktypes.h"
#include "ktest.h"

uint8_t			    gdt_ptr[6];	               
struct descriptor   gdt[GDT_SIZE];
uint8_t			    idt_ptr[6];	               
struct gate			idt[IDT_SIZE];
struct tss          tss;                        // only one tss in kernel, resides in mem, and it has a descriptor in gdt.
struct proc*        p_proc_ready;               // points to next about to run process's pcb in proc_table
struct proc         proc_table[MAX_TASKS_NUM];  // contains array of process control block: proc
												// you can think each proc entry contains a private stack for process in kernel
struct task         task_table[MAX_TASKS_NUM]={ // task_table includes sub data from proc_table
					{test_a, STACK_SIZE_TESTA, "TestA"},
					{test_b, STACK_SIZE_TESTB, "TestB"},
					{test_c, STACK_SIZE_TESTC, "TestC"}
					};

syscall_t           syscall_table[NUM_SYS_CALL] = {sys_get_ticks_impl};

// task stack is a mem area divided into MAX_TASK_NUM small areas
// each small area used as stack for a process/task
char                task_stack[STACK_SIZE_TOTAL];
pf_irq_handler_t    irq_table[IRQ_NUM];
int k_reenter = 0;

#endif