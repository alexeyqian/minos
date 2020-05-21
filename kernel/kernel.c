#include "kernel.h"
#include "asm_util.h"
#include "klib.h"
#include "const.h"
#include "ktypes.h"
#include "interrupt.h"
#include "func_def.h"
#include "ktest.h"

// global vars
uint8_t			    gdt_ptr[6];	               
struct descriptor   gdt[GDT_SIZE];
uint8_t			    idt_ptr[6];	               
struct gate			idt[IDT_SIZE];
struct tss          tss;                        // only one tss in kernel, resides in mem, and it has a descriptor in gdt.
struct proc*        p_proc_ready;               // points to next about to run process's pcb in proc_table
struct proc         proc_table[MAX_TASKS_NUM];  // contains array of process control block: proc
												// you can think each proc entry contains a private stack for process in kernel
struct task         task_table[MAX_TASKS_NUM]={ // task_table includes sub data from proc_table
					{test_a, STACK_SIZE_TESTA, "TestA"},
					{test_b, STACK_SIZE_TESTB, "TestB"},
					{test_c, STACK_SIZE_TESTC, "TestC"}
					};

int get_ticks_impl();
syscall_t           syscall_table[SYS_CALL_COUNT] = {get_ticks_impl};

// task stack is a mem area divided into MAX_TASK_NUM small areas
// each small area used as stack for a process/task
char                task_stack[STACK_SIZE_TOTAL];
pf_irq_handler_t    irq_table[IRQ_NUM];
int k_reenter;
int ticks;

void replace_gdt();
void init_tss();
void init_descriptor(struct descriptor* p_desc, uint32_t base, uint32_t limit, uint16_t attribute);
void init_ldt_descriptors_in_dgt();
void init_proc_table_from_task_table();

void put_irq_handler(int irq, pf_irq_handler_t handler);
void clock_handler(int irq);
uint32_t seg_to_physical(uint16_t seg);
void init_timer();

void delay(int time);
void restart();

void kinit(){
    //clear_screen();
    kprint("\n----- kinit begin -----\n");
	replace_gdt();  
	init_idt();
	init_tss();	
	init_ldt_descriptors_in_dgt();

	load_gdt(); // load new kernel gdt
	load_idt();		
	kprint("\n----- kinit end -----\n");
}

void irq_handler(int irq);
void kmain(){ // entrance of process
	kprint("\n -------- kmain begin --------- \n");	
	// load_tss(); // run in entry.asm 
	init_proc_table_from_task_table();	

	// TODO: move to some where??
	int i;
	for(i = 0; i < IRQ_NUM; i++)
		irq_table[i] = irq_handler;

	init_timer();

	put_irq_handler(CLOCK_IRQ, clock_handler);
	enable_irq(CLOCK_IRQ);			

	restart();
	while(1){}
}

void replace_gdt(){
	store_gdt(); // store old gdt to gdt_ptr

	// copy gdt from loader mem area to kernel mem area for kernel access
    memcpy((char*)&gdt,                         // new gdt
        (char*)(*((uint32_t*)(&gdt_ptr[2]))),   // base  of old GDT
		*((uint16_t*)(&gdt_ptr[0])) + 1	        // limit of old GDT
	);

	// update the gdt_ptr
	uint32_t* p_gdt_base  = (uint32_t*)(&gdt_ptr[2]);
    uint16_t* p_gdt_limit = (uint16_t*)(&gdt_ptr[0]);
	*p_gdt_base  = (uint32_t)&gdt;
	*p_gdt_limit = GDT_SIZE * sizeof(struct descriptor) - 1;
}

void init_tss(){
	memset((char*)&tss, 0, sizeof(tss));
	tss.ss0 = SELECTOR_KERNEL_DATA;
	init_descriptor((struct descriptor*)&gdt[INDEX_TSS],
			virtual_to_physical(seg_to_physical(SELECTOR_KERNEL_DATA), &tss),
			sizeof(tss) - 1,
			DA_386TSS);	
	tss.iobase	= sizeof(tss);	// NO IOPL map
}

// each process has one ldt descriptor in gdt.
void init_ldt_descriptors_in_dgt(){
	int i;
	struct proc* p_proc = proc_table;
	uint16_t selector_ldt = INDEX_LDT_FIRST << 3; 
	for(i = 0; i < MAX_TASKS_NUM; i++){
		// Fill LDT descriptor in GDT
		init_descriptor((struct descriptor*)&gdt[selector_ldt >> 3],
				virtual_to_physical(seg_to_physical(SELECTOR_KERNEL_DATA), proc_table[i].ldts),
				LDT_SIZE * sizeof(struct descriptor) - 1,
				DA_LDT);
		p_proc++;
		selector_ldt += 1 << 3;
	}
}

void init_proc_table_from_task_table(){
	struct proc* p_proc = proc_table;
	struct task* p_task = task_table;
	//TODO:  p_task_stack rename to process_inner_stack??
	char* p_task_stack  = task_stack + STACK_SIZE_TOTAL;
	uint16_t selector_ldt = SELECTOR_LDT_FIRST;

	// initialize proc_table according to task_table
	// each process has a ldt selector points to a ldt descriptor in GDT.
	int i;
	for(i = 0; i < MAX_TASKS_NUM; i++){
		strcpy(p_proc->p_name, p_task->name);
		p_proc->pid = 1;

		// init process ldt selector, which points to a ldt descriptor in gdt.
		p_proc->ldt_sel = selector_ldt;

		// init process ldt, which contains two ldt descriptors.
		memcpy(
			(char*)&p_proc->ldts[0], 
			(char*)&gdt[SELECTOR_KERNEL_CODE >> 3], 
			sizeof(struct descriptor));	
		// change the DPL
		p_proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;	
	
		memcpy(
			(char*)&p_proc->ldts[1], 
			(char*)&gdt[SELECTOR_KERNEL_DATA >> 3], 
			sizeof(struct descriptor));
		// change the DPL
		p_proc->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;	

		// init registers
		// cs points to first LDT descriptor
		// ds, es, fs, ss point to sencod LDT descriptor
		// gs points to video descroptor in GDT
		p_proc->regs.cs		= ((8 * 0) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		p_proc->regs.ds		= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		p_proc->regs.es		= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		p_proc->regs.fs		= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		p_proc->regs.ss		= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | RPL_TASK;
		p_proc->regs.gs		= (SELECTOR_KERNEL_VIDEO & SA_RPL_MASK) | RPL_TASK;
		p_proc->regs.eip	= (uint32_t)p_task->initial_eip;
		p_proc->regs.esp	= (uint32_t) p_task_stack; // points to seperate stack for process/task 
		p_proc->regs.eflags	= 0x1202;	// IF=1, IOPL=1, bit 2 is always 1.
	
		p_task_stack -= p_task->stack_size;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;	
	}
	k_reenter = 0;	
	ticks = 0;
	p_proc_ready = proc_table; 

	proc_table[0].ticks = proc_table[0].priority = 150;	
	proc_table[1].ticks = proc_table[1].priority = 50;	
	proc_table[2].ticks = proc_table[2].priority = 30;	
}

// from segment to physical address
uint32_t seg_to_physical(uint16_t seg){
	struct descriptor* p_dest = &gdt[seg >> 3];
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
		for( p = proc_table; p < proc_table + MAX_TASKS_NUM; p++)
			if(p->ticks > greatest_ticks){
				greatest_ticks = p->ticks;
				p_proc_ready = p;
			}

		if(!greatest_ticks)
			for(p = proc_table; p < proc_table + MAX_TASKS_NUM; p++)
				p->ticks = p->priority;
	}
}

void clock_handler(int irq){
	kprint("[");

	ticks++;
	p_proc_ready->ticks--;

	if(k_reenter != 0){
		kprint("!");
		return;
	}

	//if (p_proc_ready->ticks > 0) return;

	schedule();

	kprint("]");
}

/* round robin version of scheduler
void clock_handler(int irq){
	kprint("[");

	ticks++;
	if(k_reenter != 0){
		kprint("!");
		return;
	}

	// round robin process scheduler.
	p_proc_ready++;
	if(p_proc_ready >= proc_table + MAX_TASKS_NUM)
		p_proc_ready = proc_table;

	kprint("]");
}*/

void irq_handler(int irq){
    kprint("IRQ handler: ");
	print_int_as_hex(irq);
	kprint("\n");
}

void init_timer(){ // init 8253 PIT
	out_byte(TIMER_MODE, RATE_GENERATOR);
	out_byte(TIMER0, (uint8_t) (TIMER_FREQ/HZ) );
	out_byte(TIMER0, (uint8_t) ((TIMER_FREQ/HZ) >> 8));
}

// system call implementations
int get_ticks_impl(){
	return ticks;	
}