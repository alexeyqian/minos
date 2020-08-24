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
#include "syscall.h"
#include "hd.h"
#include "fs.h"
#include "tty.h"
#include "mm.h"
#include "test.h"
#include "screen.h"

void init();

// task stack is a mem area divided into MAX_TASK_NUM small areas
// each small area used as stack for a process/task
// kernel/tasks run in ring 0, should only use tty0, kprintf, kassert, kpanic etc...
char                task_stack[STACK_SIZE_TOTAL];
struct task         task_table[NR_TASKS]={  						
						{task_sys, STACK_SIZE_SYS,   "task_sys"  },
						{task_hd,  STACK_SIZE_HD,    "task_hd"   },
						{task_fs,  STACK_SIZE_FS,    "task_fs"   },
						{task_tty, STACK_SIZE_TTY,   "task_tty"  },
						{task_mm,  STACK_SIZE_MM,    "task_mm"   },
						{task_test,STACK_SIZE_TEST,  "task_test" }
					};
struct task         user_proc_table[NR_PROCS]={ 	
						{init,     STACK_SIZE_INIT,  "init"},		
						{test_a,   STACK_SIZE_TESTA, "test_a"},
						{test_b,   STACK_SIZE_TESTB, "test_b"},
						{test_c,   STACK_SIZE_TESTC, "test_c"}
					};	

PUBLIC void init_descriptor(struct descriptor* p_desc, uint32_t base, uint32_t limit, uint16_t attribute)
{
	p_desc->limit_low	     = limit & 0x0FFFF;		         // 段界限 1		(2 字节)
	p_desc->base_low		 = base & 0x0FFFF;		         // 段基址 1		(2 字节)
	p_desc->base_mid		 = (base >> 16) & 0x0FF;		 // 段基址 2		(1 字节)
	p_desc->attr1			 = attribute & 0xFF;		     // 属性 1
	p_desc->limit_high_attr2 = ((limit >> 16) & 0x0F) | ((attribute >> 8) & 0xF0); // 段界限 2 + 属性 2
	p_desc->base_high		 = (base >> 24) & 0x0FF;		 // 段基址 3		(1 字节)
}

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
			p_proc->ldt[INDEX_LDT_C].attr1 =  DA_C   | privilege << 5;	
			p_proc->ldt[INDEX_LDT_RW].attr1 = DA_DRW | privilege << 5;				
		}else{ // init process			
			kprintf("k_base: 0x%x, k_limit: 0x%x, NR_K: %d\n", 
				g_boot_params.kernel_base, g_boot_params.kernel_limit,
				4 * ((g_boot_params.kernel_base + g_boot_params.kernel_limit) >> LIMIT_4K_SHIFT));
			
			kprintf(">>> hard code init mem to 0x%x", 256 * 4096);
			// TODO: hard code for 1M memory for init here
			// since the get kernel_Base and kernel_limit is not correct!
			init_descriptor(&p_proc->ldt[INDEX_LDT_C], 
				0, // bytes before the entry point are not used
				(PROC_IMAGE_SIZE_DEFAULT - 1) >> LIMIT_4K_SHIFT, 
				//(g_boot_params.kernel_base + g_boot_params.kernel_limit) >> LIMIT_4K_SHIFT,
				DA_32 | DA_LIMIT_4K | DA_C | privilege << 5);

			init_descriptor(&p_proc->ldt[INDEX_LDT_RW], 
				0, // bytes before the entry point are not used
				(PROC_IMAGE_SIZE_DEFAULT - 1) >> LIMIT_4K_SHIFT, 
				//(g_boot_params.kernel_base + g_boot_params.kernel_limit) >> LIMIT_4K_SHIFT,
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

struct posix_tar_header
{				/* byte offset */
	char name[100];		/*   0 */
	char mode[8];		/* 100 */
	char uid[8];		/* 108 */
	char gid[8];		/* 116 */
	char size[12];		/* 124 */
	char mtime[12];		/* 136 */
	char chksum[8];		/* 148 */
	char typeflag;		/* 156 */
	char linkname[100];	/* 157 */
	char magic[6];		/* 257 */
	char version[2];	/* 263 */
	char uname[32];		/* 265 */
	char gname[32];		/* 297 */
	char devmajor[8];	/* 329 */
	char devminor[8];	/* 337 */
	char prefix[155];	/* 345 */
	/* 500 */
};

// ring 3
void untar(const char* filename){
	printf(">>> extract %s\n", filename);
	int fd = open(filename, O_RDWR);
	//assert(fd != -1);

	char buf[SECTOR_SIZE*16];
	int chunk = sizeof(buf);
	while(1){
		read(fd, buf, SECTOR_SIZE);
		if(buf[0] == 0) break;

		struct posix_tar_header* phdr = (struct posix_tar_header*)buf;

		// calculate file size
		char* p = phdr->size;
		int f_len = 0;
		while(*p)
			f_len = (f_len*8) + (*p++ - '0'); //octal
		
		int bytes_left = f_len;
		int fdout = open(phdr->name, O_CREAT|O_RDWR);
		if(fdout == -1){
			printf("- failed to extract file: %s\n", phdr->name);
			printf(">>> aborted\n");
			return;
		}

		printf("- %s(%d bytes\n", phdr->name, f_len);
		while(bytes_left){
			int iobytes = min(chunk, bytes_left);
			read(fd, buf, ((iobytes - 1) / SECTOR_SIZE + 1) * SECTOR_SIZE);
			write(fdout, buf, iobytes);
			bytes_left -= iobytes;
		}
		close(fdout);
	}

	close(fd);
	printf("extract done\n");
}

// <ring 3> first process, parent for all user processes.
void init(){
	kprintf(">>> 6. init is running\n");
	int fd_stdin = open("/dev_tty0", O_RDWR);
	kassert(fd_stdin == 0);
	int fd_stdout = open("/dev_tty0", O_RDWR);
	kassert(fd_stdout == 1);

	kclear_screen(); 

	// from here, the first user process init is started.
	printf(">>> first proc: init  is ready\n");

	int pid = fork();
	if(pid != 0){ // parent process
		printf(">>> [Parent] parent is running, child pid: %d\n", pid);
		int s;
		int child = wait(&s);
		printf(">>> [Parent] child %d exited with status: %d", child, s);
	}else { // child process
		printf(">>> [Child] child process is running, pid: %d\n", getpid());
		exit(123);
	}	
	// keep wating for other process to exist as transferred parents.
	while(1){
		int s;
		int child = wait(&s);
		printf("[Init] child %d exited with satus: %d\n", child, s);
	}
}