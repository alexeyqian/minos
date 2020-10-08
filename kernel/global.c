#define GLOBAL_VARIABLES_HERE
#include "global.h"
#include <const.h>
#include <sys/types.h>
#include <minos/const.h>
#include <minos/types.h>
#include "ktypes.h"
#include "ke_asm_utils.h"
#include "proto.h"

int                k_reenter;
uint32_t           ticks;
struct boot_params g_boot_params;

uint8_t			    gdt_ptr[6];	               
struct descriptor   gdt[GDT_SIZE];
uint8_t			    idt_ptr[6];	               
struct gate			idt[IDT_SIZE];
struct tss          tss;                       

struct proc*        p_proc_ready; // points to next about to run process's pcb in proc_table
struct proc         proc_table[PROCTABLE_SIZE];  // all processs include system tasks and user process

syscall_t           syscall_table[2] = {sys_kcall, sys_sendrec};