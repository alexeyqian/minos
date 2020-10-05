#include "kernel.h"

char                task_stack[STACK_SIZE_TOTAL];
struct task         task_table[NR_TASKS]={  						
						{task_clock, STACK_SIZE_CLOCK, "task_clock"},
						{task_sys,   STACK_SIZE_SYS,   "task_sys"  },
						
					};
struct task         user_proc_table[NR_PROCS]={ 	
						{drv_hd,     STACK_SIZE_HD,  "drv_hd"},
						{svc_fs,     STACK_SIZE_FS,  "svc_fs"},	
					};	

PUBLIC void init_proc_table(){
	uint8_t privilege, rpl;
	int i, prio;
	uint32_t eflags;

	struct proc* p_proc = proc_table;
	struct task* p_task; 
	char* p_task_stack  = task_stack + STACK_SIZE_TOTAL;

	// initialize proc_table according to task table
	// each process has a ldt selector points to a ldt descriptor in GDT.
	for(i = 0; i < NR_TASKS + NR_PROCS; i++, p_proc++, p_task++){	
		if(i < NR_TASKS){ // tasks
			p_task = task_table + i;
			privilege = PRIVILEGE_TASK; // apply system task permission
			rpl = RPL_TASK;
			eflags = 0x1202; // IF=1, IOPL=1, bit2 = 1
			prio = 30;
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
			// init process ldt, which contains two ldt descriptors.
			p_proc->ldt[INDEX_LDT_C]  = gdt[SELECTOR_KERNEL_CODE >> 3];
			p_proc->ldt[INDEX_LDT_RW] = gdt[SELECTOR_KERNEL_DATA >> 3];			
			// change the DPL to lower privillege
			//kassert(DA_C <= UINT8_MAX);
			//kassert(DA_DRW <= UINT8_MAX);
			p_proc->ldt[INDEX_LDT_C].attr1 =  (uint8_t)(DA_C   | privilege << 5);	
			p_proc->ldt[INDEX_LDT_RW].attr1 = (uint8_t)(DA_DRW | privilege << 5);				
		}else{ // init process		
			/*	
			kprintf("k_base: 0x%x, k_limit: 0x%x, NR_K: %d\n", 
				g_boot_params.kernel_base, g_boot_params.kernel_limit,
				4 * ((g_boot_params.kernel_base + g_boot_params.kernel_limit) >> LIMIT_4K_SHIFT));
			
			kprintf(">>> hard code init mem to 0x%x\n", 256 * 4096);
			
			// TODO: hard code for 1M memory for init here
			// since the get kernel_Base and kernel_limit is not correct!
			init_descriptor(&p_proc->ldt[INDEX_LDT_C], 
				0, // bytes before the entry point are not used
				(PROC_IMAGE_SIZE_DEFAULT - 1) >> LIMIT_4K_SHIFT, 
				//(g_boot_params.kernel_base + g_boot_params.kernel_limit) >> LIMIT_4K_SHIFT,
				(uint16_t)(DA_32 | DA_LIMIT_4K | DA_C | privilege << 5));

			init_descriptor(&p_proc->ldt[INDEX_LDT_RW], 
				0, // bytes before the entry point are not used
				(PROC_IMAGE_SIZE_DEFAULT - 1) >> LIMIT_4K_SHIFT, 
				//(g_boot_params.kernel_base + g_boot_params.kernel_limit) >> LIMIT_4K_SHIFT,
				(uint16_t)(DA_32 | DA_LIMIT_4K | DA_DRW | privilege << 5));				*/
		}

		// init registers
		// cs points to first LDT descriptor: code descriptor
		// ds, es, fs, ss point to sencod LDT descriptor: data descriptor
		// gs still points to video descroptor in GDT, but with updated lower privillege level
		p_proc->regs.cs		= INDEX_LDT_C << 3 | SA_TIL | rpl;
		p_proc->regs.ds		= p_proc->regs.es = p_proc->regs.fs = p_proc->regs.ss
							= INDEX_LDT_RW << 3 | SA_TIL | rpl;
		p_proc->regs.gs		= (SELECTOR_KERNEL_VIDEO & SA_RPL_MASK) | rpl;
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
		
		p_task_stack -= p_task->stack_size;		
	}

	k_reenter = 0;	
	ticks = 0;
	p_proc_ready = proc_table; // set default ready process
}
