#include "proc.h"
#include "const.h"
#include "types.h"
#include "ktypes.h"
#include "string.h"
#include "klib.h"
#include "ipc.h"
#include "ktest.h"

// TODO: replace externs with header inclusion
extern void task_tty();
extern void task_sys();
extern void task_hd();
extern void task_fs();

// task stack is a mem area divided into MAX_TASK_NUM small areas
// each small area used as stack for a process/task
char                task_stack[STACK_SIZE_TOTAL];
struct task         task_table[NR_TASKS]={ 
						{task_tty, STACK_SIZE_TTY,   "task_tty"  },
						{task_sys, STACK_SIZE_SYS,   "task_sys"  },
						{task_hd,  STACK_SIZE_HD,    "task_hd"   },
						{task_fs,  STACK_SIZE_FS,    "task_fs"   }
					};
struct task         user_proc_table[NR_PROCS]={ 					
						{test_a,   STACK_SIZE_TESTA, "TestA"},
						{test_b,   STACK_SIZE_TESTB, "TestB"},
						{test_c,   STACK_SIZE_TESTC, "TestC"}
					};	

PUBLIC void init_proc_table(){
	uint8_t privilege;
	uint8_t rpl;
	int eflags;
	int prio = 5;

	struct proc* p_proc = proc_table;
	struct task* p_task = task_table;
	char* p_task_stack  = task_stack + STACK_SIZE_TOTAL;
	uint16_t selector_ldt = SELECTOR_LDT_FIRST;

	// initialize proc_table according to task table
	// each process has a ldt selector points to a ldt descriptor in GDT.
	int i;
	for(i = 0; i < NR_TASKS + NR_PROCS; i++){
		
		if(i < NR_TASKS){ // tasks
			p_task = task_table + i;
			privilege = PRIVILEGE_TASK; // apply system task permission
			rpl = RPL_TASK;
			eflags = 0x1202; // IF=1, IOPL=1, bit2 = 1
		}else{ // user processes
			p_task = user_proc_table + (i - NR_TASKS);
			privilege = PRIVILEGE_USER; // apply user process permission
			rpl = RPL_USER;
			eflags = 0x202; // IF=1, bit2 = 1, remove IO permission for user process
		}

		strcpy(p_proc->p_name, p_task->name);
		p_proc->pid = 1;

		// init process ldt selector, which points to a ldt descriptor in gdt.
		p_proc->ldt_sel = selector_ldt;

		// init process ldt, which contains two ldt descriptors.
		memcpy(
			(char*)&p_proc->ldt[0], 
			(char*)&gdt[SELECTOR_KERNEL_CODE >> 3], 
			sizeof(struct descriptor));	
		// change the DPL to lower privillege
		p_proc->ldt[0].attr1 = DA_C | privilege << 5;	
	
		memcpy(
			(char*)&p_proc->ldt[1], 
			(char*)&gdt[SELECTOR_KERNEL_DATA >> 3], 
			sizeof(struct descriptor));
		// change the DPL to lower privillege
		p_proc->ldt[1].attr1 = DA_DRW | privilege << 5;	

		// init registers
		// cs points to first LDT descriptor: code descriptor
		// ds, es, fs, ss point to sencod LDT descriptor: data descriptor
		// gs still points to video descroptor in GDT, but with updated lower privillege level
		p_proc->regs.cs		= ((8 * 0) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ds		= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.es		= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.fs		= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ss		= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.gs		= (SELECTOR_KERNEL_VIDEO & SA_RPL_MASK) | rpl;
		p_proc->regs.eip	= (uint32_t)p_task->initial_eip; // entry point for procress/task
		p_proc->regs.esp	= (uint32_t) p_task_stack; // points to seperate stack for process/task 
		p_proc->regs.eflags	= eflags;
	
		p_proc->p_flags = 0;
		p_proc->p_msg = 0;
		p_proc->p_recvfrom = NO_TASK;
		p_proc->p_sendto = NO_TASK;
		p_proc->has_int_msg = 0;
		p_proc->q_sending = 0;
		p_proc->next_sending = 0;

		for(size_t j = 0; j < NR_FILES; j++)
			p_proc->filp[j] = 0;

		p_task_stack -= p_task->stack_size;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;	

		p_proc->ticks = p_proc->priority = prio;
	}
	k_reenter = 0;	
	ticks = 0;
	p_proc_ready = proc_table; // set default ready process
}