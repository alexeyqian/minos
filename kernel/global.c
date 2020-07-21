#define GLOBAL_VARIABLES_HERE
#include "global.h"
#include "const.h"
#include "types.h"
#include "ktypes.h"

extern int sys_get_ticks();
extern int sys_write(char* buf, int len, struct proc* p_proc);
extern int sys_sendrec(int function, int src_dest, struct s_message* m, struct proc* p);
extern int sys_printx(int _unused1, int _unused2, char* s, struct proc* p_proc);

int ticks;
int k_reenter;

uint8_t			    gdt_ptr[6];	               
struct descriptor   gdt[GDT_SIZE];
uint8_t			    idt_ptr[6];	               
struct gate			idt[IDT_SIZE];
struct tss          tss;                        // only one tss in kernel, resides in mem, and it has a descriptor in gdt.

struct proc*        p_proc_ready;               // points to next about to run process's pcb in proc_table
struct proc         proc_table[NR_TASKS + NR_PROCS];  // contains array of process control block: proc

pf_irq_handler_t    irq_table[IRQ_NUM];
syscall_t           syscall_table[NR_SYSCALLS] = {sys_get_ticks, sys_printx, sys_write, sys_sendrec};