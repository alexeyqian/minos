#include "kernel.h"
#include "ke_asm_utils.h"
#include "klib.h"
#include "const.h"
#include "ktypes.h"
#include "interrupt.h"
#include "keyboard.h"
#include "func_def.h"
#include "ktest.h"
#include "shared.h"
#include "tty.h"
#include "phys_mem.h"
#include "virt_mem.h"

// linear address to physical address
#define before_paging_segbase_plus_offset(seg_base, offset) (uint32_t)(((uint32_t)seg_base) + (uint32_t)(offset))

// global vars
uint8_t			    gdt_ptr[6];	               
struct descriptor   gdt[GDT_SIZE];
uint8_t			    idt_ptr[6];	               
struct gate			idt[IDT_SIZE];
struct tss          tss;                        // only one tss in kernel, resides in mem, and it has a descriptor in gdt.
struct proc*        p_proc_ready;               // points to next about to run process's pcb in proc_table
struct proc         proc_table[TASKS_NUM + PROCS_NUM];  // contains array of process control block: proc
struct task         task_table[TASKS_NUM]={ 
						{task_tty, STACK_SIZE_TTY,   "tty"  }				
					};

// TODO: in future, os should dynamically create user process table
struct task         user_proc_table[PROCS_NUM]={ 					
						{test_a,   STACK_SIZE_TESTA, "TestA"},
						{test_b,   STACK_SIZE_TESTB, "TestB"},
						{test_c,   STACK_SIZE_TESTC, "TestC"}
					};	

TTY tty_table[NR_CONSOLES];

int write_impl(char* buf, int len, struct proc* p_proc);
int get_ticks_impl();
syscall_t           syscall_table[SYSCALLS_NUM] = {get_ticks_impl, write_impl};

// task stack is a mem area divided into MAX_TASK_NUM small areas
// each small area used as stack for a process/task
char                task_stack[STACK_SIZE_TOTAL];
pf_irq_handler_t    irq_table[IRQ_NUM];
int k_reenter;
int ticks;

void init_phys_mem();
void init_virt_mem();
void init_new_gdt();
void init_tss();
void init_descriptor(struct descriptor* p_desc, uint32_t base, uint32_t limit, uint16_t attribute);
void init_ldt_descriptors_in_dgt();
void init_proc_table();

void clock_handler(int irq);
uint32_t before_paging_selector_to_segbase(uint16_t selector);
void enable_clock();

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

	init_phys_mem();	
	//init_virt_mem();
	 
	kmain();
}

void kmain(){ 
	kprint(">>> kmain begin ... \n");		
	enable_clock(); 	
	restart(); // pretenting a schedule happend to start a process.
	while(1){}
}

void init_phys_mem(){
	// TODO: get boot info
	//boot_info* binfo_ptr = (boot_info*)(LOADER_PHYSICAL_BASE + ??)

	// memory size
	// TODO: replace hardcodes
	//uint32_t mem_size = binfo_ptr->mem_size;
	uint32_t mem_size = 0x00A00000; // 10M, need 320 bytes for mem map
	//uint32_t kenel_size = binfo_ptr->kernel_size; 
	// place the memory map of physical memory manager at end of kernel
	//pmmgr_init(mem_size, KERNEL_BIN_SEG_BASE + kernel_size);
	uint32_t mem_map_ptr = 0x500;
	pmmgr_init(mem_size, mem_map_ptr);
	//kprint("physical memory manager initilized with %i\n", mem_size);
	// TODO: replace hard code
	//mem_region* regions = (memory_region*)0x1000; // get region map from loader
	
	struct mem_region r1;
	r1.start_low = 0x00100000; // start from 1M
	r1.start_high = 0x0;
	r1.size_low = 0x00900000;  // size 9M
	r1.size_high = 0x0;
	r1.type = 1;
	
	struct mem_region regions[MEM_REGION_COUNT];
	regions[0] = r1;
	/*
	char* mem_types_str[] = {
		{"available"},
		{"reserved"},
		{"acpi reclaim"},
		{"acpi nvs memory"}
	};*/

	for(int i = 0; i < MEM_REGION_COUNT; i++){
		if(regions[i].type > 4) regions[i].type = 1; // sanity check		
		if(i > 0 && regions[i].start_low == 0) break; // no more entries

		// print region ...
		// print region %i: start: length: type:
		if(regions[i].type == 1)
			pmmgr_init_region(regions[i].start_low, regions[i].size_low);			
	}

	// mark boot region, loader region, kernel.bin region, and kernel region as used.
	// pmmgr_uninit_region(-x10000, kernel_size*512);
	// TODO: replace hard code.
	pmmgr_uninit_region(0x0, 0x100000); // reserver lower 1M

	// print i% regions initialized: %i max blocks, %i used blocks
	/*
	// TODO: test code
	uint32_t* p1 = (uint32_t*) pmmgr_alloc_block();
	kprint("p1 allocated at: ");
	kprint_int_as_hex((int)p1);
	
	uint32_t* p2 = (uint32_t*)pmmgr_alloc_block();
	kprint("p2 allocated at: ");
	kprint_int_as_hex((int)p2);	

	pmmgr_free_block(p1);
	pmmgr_free_block(p2);*/

}

void init_virt_mem(){	
	vmmgr_init();
	kprint(">>> virtual memory initialized and paging enabled.");
}

void irq_handler(int irq);

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
	for(i = 0; i < TASKS_NUM + PROCS_NUM; i++){
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

void init_proc_table(){
	uint8_t privilege;
	uint8_t rpl;
	int eflags;

	struct proc* p_proc = proc_table;
	struct task* p_task = task_table;
	char* p_task_stack  = task_stack + STACK_SIZE_TOTAL;
	uint16_t selector_ldt = SELECTOR_LDT_FIRST;

	// initialize proc_table according to task_table
	// each process has a ldt selector points to a ldt descriptor in GDT.
	int i;
	for(i = 0; i < TASKS_NUM + PROCS_NUM; i++){
		
		if(i < TASKS_NUM){ // tasks
			p_task = task_table + i;
			privilege = PRIVILEGE_TASK; // apply system task permission
			rpl = RPL_TASK;
			eflags = 0x1202; // IF=1, IOPL=1, bit2 = 1
		}else{ // user processes
			p_task = user_proc_table + (i - TASKS_NUM);
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
	
		p_proc->tty_idx = 0;
		p_task_stack -= p_task->stack_size;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;	
	}
	k_reenter = 0;	
	ticks = 0;
	p_proc_ready = proc_table; // set default ready process

	proc_table[0].ticks = proc_table[0].priority = 10;	
	proc_table[1].ticks = proc_table[1].priority = 5;	
	proc_table[2].ticks = proc_table[2].priority = 5;	
	proc_table[3].ticks = proc_table[3].priority = 5;	

	proc_table[1].tty_idx = 1; // proc_table[0] is system task, no need terminal
	proc_table[2].tty_idx = 1;
	proc_table[3].tty_idx = 1;
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

void put_irq_handler(int irq, pf_irq_handler_t handler){
	disable_irq(irq);
	irq_table[irq] = handler;
}

// priority is fixed value, ticks is counting down.
// when all processes ticks are 0, then reset ticks to it's priority.
void schedule(){
	struct proc* p;
	int greatest_ticks = 0;
	while(!greatest_ticks){
		for( p = proc_table; p < proc_table + TASKS_NUM + PROCS_NUM; p++)
			if(p->ticks > greatest_ticks){
				greatest_ticks = p->ticks;
				p_proc_ready = p;
			}

		if(!greatest_ticks)
			for(p = proc_table; p < proc_table + TASKS_NUM + PROCS_NUM; p++)
				p->ticks = p->priority;
	}
}

void clock_handler(int irq){
	//kprint("[");

	ticks++;
	p_proc_ready->ticks--;

	if(k_reenter != 0){ // interrupt re-enter
		kprint(".");
		return;
	}

	//if (p_proc_ready->ticks > 0) return;
	schedule(); 
	//kprint("]");
}

// round robin version of scheduler
void clock_handler2(int irq){
	//kprint("[");

	ticks++;
	if(k_reenter != 0){
		//kprint("!");
		return;
	}

	// round robin process scheduler.
	p_proc_ready++;
	if(p_proc_ready >= proc_table + TASKS_NUM + PROCS_NUM)
		p_proc_ready = proc_table;

	//kprint("]");
}

void irq_handler(int irq){
    kprint("IRQ handler: ");
	kprint_int_as_hex(irq);
}

void enable_clock(){ // init 8253 PIT
	out_byte(TIMER_MODE, RATE_GENERATOR);
	out_byte(TIMER0, (uint8_t) (TIMER_FREQ/HZ) );
	out_byte(TIMER0, (uint8_t) ((TIMER_FREQ/HZ) >> 8));

	put_irq_handler(CLOCK_IRQ, clock_handler);
	enable_irq(CLOCK_IRQ);	
}

// system call implementations
int get_ticks_impl(){
	return ticks;	
}

void write_to_tty(TTY* p_tty, char* buf, int len){
	char* p = buf;
	int i = len;
	while(i){
		tty_output_char(p_tty->p_console, *p++);
		i--;
	}
}

int write_impl(char* buf, int len, struct proc* p_proc){
	write_to_tty(&tty_table[p_proc->tty_idx], buf, len);
	return 0;
}