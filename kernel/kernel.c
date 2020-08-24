#include "kernel.h"

#include "const.h"
#include "types.h"
#include "ktypes.h"
#include "global.h"
#include "ke_asm_utils.h"
#include "syscall.h"
#include "string.h"
#include "assert.h"
#include "klib.h"
#include "kio.h"
#include "boot_params.h"

#include "interrupt.h"
#include "clock.h"
#include "keyboard.h" // TODO: remove, already have tty
#include "tty.h"
#include "phys_mem.h"
#include "virt_mem.h"
#include "ipc.h"
#include "screen.h" 
#include "proc.h"
#include "ktest.h"

 
// Check if the compiler thinks you are targeting the wrong operating system.
//#if defined(__linux__)
//#error "You are not using a cross-compiler, you will most certainly run into trouble"
//#endif
 
// This tutorial will only work for the 32-bit ix86 targets.
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

// linear address to physical address
#define before_paging_segbase_plus_offset(seg_base, offset) (uint32_t)(((uint32_t)seg_base) + (uint32_t)(offset))

void init_new_gdt();
void init_tss();
void init_descriptor(struct descriptor* p_desc, uint32_t base, uint32_t limit, uint16_t attribute);
void init_ldt_descriptors_in_dgt();

uint32_t before_paging_selector_to_segbase(uint16_t selector);

void restart();
void kmain();

void kstart(){
	kclear_screen();
	kprintf(">>> kstart\n");
	    
	init_new_gdt();  
	init_idt();
	init_tss();	
	init_ldt_descriptors_in_dgt(); 
	get_boot_params(&g_boot_params); // has to be before init proc table
	init_proc_table();	
	
	//pmmgr_init();	
	//vmmgr_init();kprintf(">>> virtual memory initialized and paging enabled.");
	 
	kmain();
}

void kmain(){ 
	kprintf(">>> kmain\n");	// kprintf SHOULD NOT BE USED ANYMORE AFTER THIS	
	init_clock(); 
	init_keyboard();    	
	restart(); // pretenting a schedule happend to start a process.
	// process start scheduling at this point.
	while(1){}
}

void init_new_gdt(){
	store_gdt(); // store old gdt to [gdt_ptr]

	// copy old gdt from loader mem area to new kernel mem area for easy kernel access
    memcpy((char*)&gdt,                         // new gdt
        (char*)(*((uint32_t*)(&gdt_ptr[2]))),   // base  of old GDT
		*((uint16_t*)(&gdt_ptr[0])) + 1	        // limit of old GDT
	);

	// update the gdt_ptr
	uint32_t* p_gdt_base  = (uint32_t*)(&gdt_ptr[2]);
    uint16_t* p_gdt_limit = (uint16_t*)(&gdt_ptr[0]);
	*p_gdt_base  = (uint32_t)&gdt;
	*p_gdt_limit = GDT_SIZE * sizeof(struct descriptor) - 1;

	load_gdt(); // load new kernel gdt
}

void init_tss(){
	memset((char*)&tss, 0, sizeof(tss));
	tss.ss0 = SELECTOR_KERNEL_DATA;
	// setup single tss descriptor in gdt
	init_descriptor((struct descriptor*)&gdt[INDEX_TSS],
			before_paging_segbase_plus_offset(
				before_paging_selector_to_segbase(SELECTOR_KERNEL_DATA), // should = 0x0
				&tss
			),
			sizeof(tss) - 1,
			DA_386TSS);	
	tss.iobase	= sizeof(tss);	// NO IOPL map

	load_tss();
}

// each process has one ldt descriptor in gdt.
void init_ldt_descriptors_in_dgt(){
	for(int i = 0; i < NR_TASKS + NR_PROCS; i++){
		memset(&proc_table[i], 0, sizeof(struct proc));

		proc_table[i].ldt_sel = SELECTOR_LDT_FIRST + (i << 3);
		kassert(INDEX_LDT_FIRST + i < GDT_SIZE);

		// Fill LDT descriptor in GDT
		init_descriptor(&gdt[INDEX_LDT_FIRST+i],
				before_paging_segbase_plus_offset(
					before_paging_selector_to_segbase(SELECTOR_KERNEL_DATA), // should = 0x0
					proc_table[i].ldt // physical address of LDT
				),
				LDT_SIZE * sizeof(struct descriptor) - 1,
				DA_LDT); 
	}
}

uint32_t before_paging_selector_to_segbase(uint16_t selector){
	struct descriptor* p_dest = &gdt[selector >> 3];
	return (p_dest->base_high << 24) | (p_dest->base_mid << 16) | (p_dest->base_low);
}

void shell(const char* tty_name){
	int fd_stdin = open(tty_name, O_RDWR);
	kassert(fd_stdin == FD_STDIN);
	int fd_stdout = open(tty_name, O_RDWR);
	kassert(fd_stdout == FD_STDOUT);

	char rdbuf[128];
	while(1){
		write(FD_STDOUT, "$ ", 2);
		int r = read(FD_STDIN, rdbuf, 70);
		rdbuf[r] = 0;

		int argc = 0;
		char* argv[PROC_ORIGIN_STACK];
		char* p = rdbuf;
		char* s;
		int word = 0;
		char ch;
		do{
			ch = *p;

			if(*p != ' ' && *p != 0 && !word){ // begin word
				s = p;
				word = 1;
			}

			if((*p == ' ' || *p == 0) && word){ // end word
				word = 0;
				argv[argc++] = s;
				*p = 0;
			}

			p++;
		}while(ch);

		argv[argc] = 0;

		int fd = open(argv[0], O_RDWR);
		if(fd == -1){
			if(rdbuf[0]){
				write(FD_STDOUT, rdbuf, r);
				write(FD_STDOUT, '\n', 1);
			}
		}else{
			close(fd);
			int pid = fork();
			if(pid != 0) { // parent
				int s;
				wait(&s);
			}else{ // child
				execv(argv[0], argv);
			}
		}
	}

	close(FD_STDOUT);
	close(FD_STDIN);
}