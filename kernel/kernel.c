#include "kernel.h"
#include "klib.h"
#include "const.h"
#include "interrupt.h"
#include "func_def.h"
#include "ktest.h"

#include "global_vars.h"

void kinit(){
    clear_screen();
    kprint("\n----- kinit begin -----\n");

	move_gdt_from_loader_mem_to_kernel_mem();   
	init_idt();
	init_tss();
	init_tss_descriptor_in_gdt();
	init_ldt_descriptors_in_dgt();
	init_proc_table_from_task_table();	
	kprint("\n----- kinit end -----\n");
}

void kmain(){ // entrance of process
	kprint("\n -------- kmain begin --------- \n");	
	k_reenter = 0;	

	put_irq_handler(CLOCK_IRQ, clock_handler);
	enable_irq(CLOCK_IRQ);			

	restart();
	while(1){}
}

void move_gdt_from_loader_mem_to_kernel_mem(){
	// move gdt from loader to kernel
	// and update the gdt_ptr
	// Q: why doing this ?
	// A: since previous gdt is loaded in loader mem area
	// now need to copy it to kernel mem area and uptdate the gdt_ptr
	// so the gdt will be in kernel memory area
    memcpy((char*)&gdt,                         // new gdt
        (char*)(*((uint32_t*)(&gdt_ptr[2]))),   // base  of old GDT
		*((uint16_t*)(&gdt_ptr[0])) + 1	        // limit of old GDT
	);

	uint32_t* p_gdt_base  = (uint32_t*)(&gdt_ptr[2]);
    uint16_t* p_gdt_limit = (uint16_t*)(&gdt_ptr[0]);
	*p_gdt_base  = (uint32_t)&gdt;
	*p_gdt_limit = GDT_SIZE * sizeof(struct descriptor) - 1;	

	// init idt_ptr
    // idt_ptr[6] 6 bytes in tatal：0~15:Limit  16~47:Base
	uint16_t* p_idt_limit = (uint16_t*)(&idt_ptr[0]);
	uint32_t* p_idt_base  = (uint32_t*)(&idt_ptr[2]);
	*p_idt_limit = IDT_SIZE * sizeof(struct gate) - 1;
	*p_idt_base  = (uint32_t)&idt;
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
	p_proc_ready = proc_table; 	
}

void init_tss(){
	memset((char*)&tss, 0, sizeof(tss));
	tss.ss0 = SELECTOR_KERNEL_DATA;
	tss.iobase	= sizeof(tss);	// NO IOPL map
}

// used for jump from ring1 to ring0
// most important change: ss and esp
// Q: when ss0 is initialized/setup?
void init_tss_descriptor_in_gdt(){	
	init_descriptor((struct descriptor*)&gdt[INDEX_TSS],
			virtual_to_physical(seg_to_physical(SELECTOR_KERNEL_DATA), &tss),
			sizeof(tss) - 1,
			DA_386TSS);	
}

// each process has one ldt descriptor in gdt.
void init_ldt_descriptors_in_dgt(){
	int i;
	struct proc* p_proc = proc_table;
	uint16_t selector_ldt = INDEX_LDT_FIRST << 3; // ??
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