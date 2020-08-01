#define GLOBAL_VARIABLES_HERE
#include "global.h"
#include "const.h"
#include "types.h"
#include "ktypes.h"
#include "syscall.h"
#include "fs.h"

int ticks;
int k_reenter;
uint32_t disp_pos = 0;

uint8_t			    gdt_ptr[6];	               
struct descriptor   gdt[GDT_SIZE];
uint8_t			    idt_ptr[6];	               
struct gate			idt[IDT_SIZE];
struct tss          tss;                        // only one tss in kernel, resides in mem, and it has a descriptor in gdt.

struct proc*        p_proc_ready;               // points to next about to run process's pcb in proc_table
struct proc         proc_table[NR_TASKS + NR_PROCS];  // contains array of process control block: proc

pf_irq_handler_t    irq_table[IRQ_NUM];
syscall_t           syscall_table[NR_SYSCALLS] = {sys_printx, sys_sendrec};

// remember to modify include/const.h if the order is changed
PUBLIC struct dev_drv_map dd_map[] = {
    {INVALID_DRIVER}, // unused
    {INVALID_DRIVER}, // reserved for floppy driver
    {INVALID_DRIVER}, // reserved for cdrom
    {TASK_HD},        // hard disk: driver is task_hd
    {TASK_TTY},       // tty
    {INVALID_DRIVER}  // reserved for scsi disk driver
};

// 6M-7M buffer for fs
// TODO: rename to g_fs_buf
PUBLIC uint8_t* fsbuf = (uint8_t*)0x600000;
PUBLIC const int FSBUF_SIZE = 0x100000;