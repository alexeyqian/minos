#define GLOBAL_VARIABLES_HERE
#include "global.h"

#include "types.h"
#include "ktypes.h"

extern void task_tty();
extern void task_sys();

extern int sys_get_ticks();
extern int sys_write(char* buf, int len, struct proc* p_proc);
extern int sys_sendrec(int function, int src_dest, MESSAGE* m, struct proc* p);

extern void test_a();
extern void test_b();
extern void test_c();


#define NR_SYSCALLS 4

int ticks;
int k_reenter;

uint8_t			    gdt_ptr[6];	               
struct descriptor   gdt[GDT_SIZE];
uint8_t			    idt_ptr[6];	               
struct gate			idt[IDT_SIZE];
struct tss          tss;                        // only one tss in kernel, resides in mem, and it has a descriptor in gdt.

struct proc*        p_proc_ready;               // points to next about to run process's pcb in proc_table
struct proc         proc_table[NR_TASKS + NR_PROCS];  // contains array of process control block: proc

struct task         task_table[NR_TASKS]={ 
						{task_tty, STACK_SIZE_TTY,   "task_tty"  },
						{task_sys, STACK_SIZE_SYS,   "task_sys"  }								
					};
struct task         user_proc_table[NR_PROCS]={ 					
						{test_a,   STACK_SIZE_TESTA, "TestA"},
						{test_b,   STACK_SIZE_TESTB, "TestB"},
						{test_c,   STACK_SIZE_TESTC, "TestC"}
					};	
// task stack is a mem area divided into MAX_TASK_NUM small areas
// each small area used as stack for a process/task
char                task_stack[STACK_SIZE_TOTAL];
TTY                 tty_table[NR_CONSOLES];
syscall_t           syscall_table[NR_SYSCALLS] = {sys_get_ticks, sys_write, sys_printx, sys_sendrec};
pf_irq_handler_t    irq_table[IRQ_NUM];
