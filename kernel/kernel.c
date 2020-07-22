#include "kernel.h"

#include "const.h"
#include "types.h"
#include "ktypes.h"
#include "global.h"
#include "ke_asm_utils.h"
#include "syscall.h"
#include "klib.h"

#include "interrupt.h"
#include "clock.h"
#include "keyboard.h" // TODO: remove, already have tty
#include "tty.h"
#include "phys_mem.h"
#include "virt_mem.h"
#include "ipc.h"
#include "screen.h" // kprint
#include "proc.h"
#include "ktest.h"


// linear address to physical address
#define before_paging_segbase_plus_offset(seg_base, offset) (uint32_t)(((uint32_t)seg_base) + (uint32_t)(offset))

void init_new_gdt();
void init_tss();
void init_descriptor(struct descriptor* p_desc, uint32_t base, uint32_t limit, uint16_t attribute);
void init_ldt_descriptors_in_dgt();

uint32_t before_paging_selector_to_segbase(uint16_t selector);

void restart();
void kmain();

void kinit(){
	//kclear_screen();
    kprint(">>> kinit begin ...\n");
	init_new_gdt();  
	init_idt();
	init_tss();	
	init_ldt_descriptors_in_dgt(); 
	init_proc_table();	 	

	//pmmgr_init();	
	//vmmgr_init();kprint(">>> virtual memory initialized and paging enabled.");
	 
	kmain();
}

void kmain(){ 
	kprint(">>> kmain begin ... \n");		
	enable_clock(); 	
	restart(); // pretenting a schedule happend to start a process.
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
	int i;
	struct proc* p_proc = proc_table;
	uint16_t selector_ldt = INDEX_LDT_FIRST << 3; 
	for(i = 0; i < NR_TASKS + NR_PROCS; i++){
		// Fill LDT descriptor in GDT
		init_descriptor((struct descriptor*)&gdt[selector_ldt >> 3],
				before_paging_segbase_plus_offset(
					before_paging_selector_to_segbase(SELECTOR_KERNEL_DATA), // should = 0x0
					proc_table[i].ldt // physical address of LDT
				),
				LDT_SIZE * sizeof(struct descriptor) - 1,
				DA_LDT); 
		p_proc++;
		selector_ldt += 1 << 3;
	}
}

uint32_t before_paging_selector_to_segbase(uint16_t selector){
	struct descriptor* p_dest = &gdt[selector >> 3];
	return (p_dest->base_high << 24) | (p_dest->base_mid << 16) | (p_dest->base_low);
}

// TODO: remove later, it's a hack for linker issue.
uint32_t __stack_chk_fail_local(){
    return 0;
}

void init_descriptor(struct descriptor* p_desc, uint32_t base, uint32_t limit, uint16_t attribute)
{
	p_desc->limit_low		= limit & 0x0FFFF;		     // 段界限 1		(2 字节)
	p_desc->base_low		= base & 0x0FFFF;		     // 段基址 1		(2 字节)
	p_desc->base_mid		= (base >> 16) & 0x0FF;		 // 段基址 2		(1 字节)
	p_desc->attr1			= attribute & 0xFF;		     // 属性 1
	p_desc->limit_high_attr2	= ((limit >> 16) & 0x0F) | (attribute >> 8) & 0xF0; // 段界限 2 + 属性 2
	p_desc->base_high		= (base >> 24) & 0x0FF;		 // 段基址 3		(1 字节)
}

// system call implementations
int sys_get_ticks(){
	return ticks;	
}

