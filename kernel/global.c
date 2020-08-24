#define GLOBAL_VARIABLES_HERE
#include "global.h"
#include "const.h"
#include "types.h"
#include "ktypes.h"
#include "ipc.h"
#include "tty.h"
#include "fs.h"

int                k_reenter;
uint32_t           ticks;
uint32_t           g_disp_pos = 0; 
struct boot_params g_boot_params;

uint8_t			    gdt_ptr[6];	               
struct descriptor   gdt[GDT_SIZE];
uint8_t			    idt_ptr[6];	               
struct gate			idt[IDT_SIZE];
struct tss          tss;                        // only one tss in kernel, resides in mem, and it has a descriptor in gdt.

struct proc*        p_proc_ready;               // points to next about to run process's pcb in proc_table
struct proc         proc_table[NR_TASKS + NR_PROCS];  // contains array of process control block: proc

pf_irq_handler_t    irq_table[IRQ_NUM];
syscall_t           syscall_table[NR_SYSCALLS] = {sys_printx, sys_sendrec};

// 6M - 7M buffer for fs
PUBLIC uint8_t*  fsbuf = (uint8_t*)0x600000;
PUBLIC const int FSBUF_SIZE = 0x100000;

// 7M - 8M buffer for mm 
PUBLIC	uint8_t*  mmbuf = (uint8_t*)0x700000;
PUBLIC	const int MMBUF_SIZE = 0x100000;
