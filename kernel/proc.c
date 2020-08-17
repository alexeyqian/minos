#include "proc.h"
#include "const.h"
#include "types.h"
#include "ktypes.h"
#include "string.h"
#include "klib.h"
#include "global.h"
#include "ipc.h"
#include "ktest.h"
#include "assert.h"
#include "boot_params.h"
#include "kio.h"

// TODO: replace externs with header inclusion
extern void task_tty();
extern void task_sys();
extern void task_hd();
extern void task_fs();
extern void task_mm();
void init();

// TODO: move to global
// task stack is a mem area divided into MAX_TASK_NUM small areas
// each small area used as stack for a process/task
char                task_stack[STACK_SIZE_TOTAL];
struct task         task_table[NR_TASKS]={ 
						{task_tty, STACK_SIZE_TTY,   "task_tty"  },
						{task_sys, STACK_SIZE_SYS,   "task_sys"  },
						{task_hd,  STACK_SIZE_HD,    "task_hd"   },
						{task_fs,  STACK_SIZE_FS,    "task_fs"   },
						{task_mm,  STACK_SIZE_MM,    "task_mm"}
					};
struct task         user_proc_table[NR_PROCS]={ 	
						{init,     STACK_SIZE_INIT,  "init"},		// TODO: hardcode -> const		
						{test_a,   STACK_SIZE_TESTA, "test_a"},
						{test_b,   STACK_SIZE_TESTB, "test_b"},
						{test_c,   STACK_SIZE_TESTC, "test_c"}
					};	

PUBLIC void init_proc_table(){
	uint8_t privilege, rpl;
	int i, j, eflags, prio;

	struct proc* p_proc = proc_table;
	struct task* p_task; 
	char* p_task_stack  = task_stack + STACK_SIZE_TOTAL;

	// initialize proc_table according to task table
	// each process has a ldt selector points to a ldt descriptor in GDT.
	for(i = 0; i < NR_TASKS + NR_PROCS; i++, p_proc++, p_task++){
		if(i >= NR_TASKS + NR_NATIVE_PROCS){
			p_proc->p_flags = FREE_SLOT;
			continue;
		}
		
		if(i < NR_TASKS){ // tasks
			p_task = task_table + i;
			privilege = PRIVILEGE_TASK; // apply system task permission
			rpl = RPL_TASK;
			eflags = 0x1202; // IF=1, IOPL=1, bit2 = 1
			prio = 15;
		}else{ // user processes
			p_task = user_proc_table + (i - NR_TASKS);
			privilege = PRIVILEGE_USER; // apply user process permission
			rpl = RPL_USER;
			eflags = 0x202; // IF=1, bit2 = 1, remove IO permission for user process
			prio = 5;
		}

		strcpy(p_proc->p_name, p_task->name);
		p_proc->p_parent = NO_TASK;

		if(strcmp(p_proc->p_name, "init") != 0){ // not init process
			kprintf("i: %d, pname: %s\n", i, p_proc->p_name);
			// init process ldt, which contains two ldt descriptors.
			p_proc->ldt[INDEX_LDT_C]  = gdt[SELECTOR_KERNEL_CODE >> 3];
			p_proc->ldt[INDEX_LDT_RW] = gdt[SELECTOR_KERNEL_DATA >> 3];
			/*
			memcpy(
				(char*)&p_proc->ldt[INDEX_LDT_C], 
				(char*)&gdt[SELECTOR_KERNEL_CODE >> 3], 
				sizeof(struct descriptor));			
			memcpy(
				(char*)&p_proc->ldt[INDEX_LDT_RW], 
				(char*)&gdt[SELECTOR_KERNEL_DATA >> 3], 
				sizeof(struct descriptor));
			*/
			// change the DPL to lower privillege
			p_proc->ldt[INDEX_LDT_C].attr1 =  DA_C   | privilege << 5;	
			p_proc->ldt[INDEX_LDT_RW].attr1 = DA_DRW | privilege << 5;				
		}else{ // init process
			kprintf("i: %d, pname: %s\n", i, p_proc->p_name);
			uint32_t k_base;
			uint32_t k_limit;
			int ret = get_kernel_map(&k_base, &k_limit, &g_boot_params);
			assert(ret == 0);
			//kprintf("k_base: 0x%x, k_limit: 0x%x\n", k_base, k_limit);
			init_descriptor(&p_proc->ldt[INDEX_LDT_C], 
				0, // bytes before the entry point are not used
				(k_base + k_limit) >> LIMIT_4K_SHIFT,
				DA_32 | DA_LIMIT_4K | DA_C | privilege << 5);

			init_descriptor(&p_proc->ldt[INDEX_LDT_RW], 
				0, // bytes before the entry point are not used
				(k_base + k_limit) >> LIMIT_4K_SHIFT,
				DA_32 | DA_LIMIT_4K | DA_DRW | privilege << 5);
		}

		// init registers
		// cs points to first LDT descriptor: code descriptor
		// ds, es, fs, ss point to sencod LDT descriptor: data descriptor
		// gs still points to video descroptor in GDT, but with updated lower privillege level
		p_proc->regs.cs		= INDEX_LDT_C << 3 | SA_TIL | rpl;
		p_proc->regs.ds		= p_proc->regs.es = p_proc->regs.fs = p_proc->regs.ss
							= INDEX_LDT_RW << 3 | SA_TIL | rpl;
		p_proc->regs.gs		= (SELECTOR_KERNEL_VIDEO & SA_RPL_MASK) | rpl;
		// TODO: replace p_task to p_proc
		p_proc->regs.eip	= (uint32_t)p_task->initial_eip; // entry point for procress/task
		p_proc->regs.esp	= (uint32_t) p_task_stack; // points to seperate stack for process/task 
		p_proc->regs.eflags	= eflags;
	
		p_proc->ticks = p_proc->priority = prio;

		p_proc->p_flags = 0;
		p_proc->p_msg = 0;
		p_proc->p_recvfrom = NO_TASK;
		p_proc->p_sendto = NO_TASK;
		p_proc->has_int_msg = 0;
		p_proc->q_sending = 0;
		p_proc->next_sending = 0;

		for(j = 0; j < NR_FILES; j++)
			p_proc->filp[j] = 0;

		p_task_stack -= p_task->stack_size;		
	}

	k_reenter = 0;	
	ticks = 0;
	p_proc_ready = proc_table; // set default ready process
}

// first process, parent for all user processes.
void init(){while(1){}
	//kprintf(">>> init proc is running ...\n");
	kspin("init");
	int fd_stdin = open("/dev_tty0", O_RDWR);
	assert(fd_stdin == 0);
	int fd_stdout = open("/dev_tty0", O_RDWR);
	assert(fd_stdout == 1);

	int pid = fork();
	if(pid != 0){ // parent process
		//kprintf(">>> parent is running, child pid: %d\n", pid);
		int s;
		int child = wait(&s);
		//kprintf("child %d exited with status: %d", child, s);
		kspin(">>> parent...\n");
	}else { // child process
		//kprintf(">>> child process is running, pid: %d\n", getpid());
		//execl("/echo", "echo", "hello", "world", 0);
		exit(123);
	}	

	while(1){
		int s;
		int child = wait(&s);
		//kprintf("child %d exited with satus: %d\n", child, s);
	}
}