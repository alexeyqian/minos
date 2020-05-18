#include "kernel.h"
#include "klib.h"

// function declaration
void init_descriptor(struct descriptor* p_desc, uint32_t base, uint32_t limit, uint16_t attribute);
void init_idt_descriptor(unsigned char vector, uint8_t desc_type, pf_int_handler_t handler, unsigned char privilege);
uint32_t seg_to_physical(uint16_t seg);
void move_gdt_from_loader_mem_to_kernel_mem();
void init_idt();
void init_tss();
void init_tss_descriptor_in_gdt();
void init_ldt_descriptors_in_dgt();
void init_proc_table_from_task_table();
void delay(int time);
void resume();
void test_a();
void test_b();
void test_c();

// global variables
uint8_t			    gdt_ptr[6];	               
struct descriptor   gdt[GDT_SIZE];
uint8_t			    idt_ptr[6];	               
struct gate			idt[IDT_SIZE];
struct tss          tss;                        // only one tss in kernel, resides in mem, and it has a descriptor in gdt.
struct proc*        p_proc_ready;               // points to next about to run process's pcb in proc_table
struct proc         proc_table[MAX_TASKS_NUM];  // contains array of process control block: proc
struct task         task_table[MAX_TASKS_NUM]={ // task_table includes sub data from proc_table
					{test_a, STACK_SIZE_TESTA, "TestA"},
					{test_b, STACK_SIZE_TESTB, "TestB"},
					{test_c, STACK_SIZE_TESTC, "TestC"}
					};

// task_stack is a mem area divided into MAX_TASK_NUM small areas
// each small area used as stack for a process/task
char                task_stack[STACK_SIZE_TOTAL];
int k_reenter = -1;

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
	//k_reenter = -1;	
	resume();
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

void delay(int time){
	int i, j, k;
	for(i = 0; i < time; i++)
		for(j = 0; j < 1000; j++)
			for(k = 0; k < 1000; k ++){}
}

// from segment to physical address
uint32_t seg_to_physical(uint16_t seg){
	struct descriptor* p_dest = &gdt[seg >> 3];
	return (p_dest->base_high << 24) | (p_dest->base_mid << 16) | (p_dest->base_low);
}

void test_a(){
	int i = 0;
	while(1){
		kprint("A");
		print_int(i++);
		kprint(".");
		delay(1);
	}
}

void test_b(){
	int i = 0x1000;
	while(1){
		kprint("B");
		print_int(i++);
		kprint(".");
		delay(1);
	}
}

void test_c(){
	int i = 0x2000;
	while(1){
		kprint("C");
		print_int(i++);
		kprint(".");
		delay(1);
	}
}

// TODO: remove later, it's a hack for linker issue.
uint32_t __stack_chk_fail_local(){
    return 0;
}

// below exception and interruption handlers are defined in kernel_entry.asm
// exception handlers
void	divide_error();
void	single_step_exception();
void	nmi();
void	breakpoint_exception();
void	overflow();
void	bounds_check();
void	inval_opcode();
void	copr_not_available();
void	double_fault();
void	copr_seg_overrun();
void	inval_tss();
void	segment_not_present();
void	stack_exception();
void	general_protection();
void	page_fault();
void	copr_error();

// interrupt handlers
void	irq00(); 
void	irq01();
void	irq02();
void	irq03();
void	irq04();
void	irq05();
void	irq06();
void	irq07();
void	irq08();
void	irq09();
void	irq10();
void	irq11();
void	irq12();
void	irq13();
void	irq14();
void	irq15();

// setup chip 8259A which is a bridge between interrupting devices and CPU
// ICW: Initialization Command Word
// there are 4 types of ICW, from ICW1 - ICW4, each has specific format
// The write operation must be ordered correctly
// In real mode, BIOS map IRQ0-IRQ7 TO 0X8 - 0XF, but in proteced mode these are occupied,
// so we need to remap them to 0x20 and 0x28
void init_8259a(){
    out_byte(INT_M_CTL,	0x11);			         // Master, ICW1.
	out_byte(INT_S_CTL,	0x11);			         // Slave , ICW1.
	out_byte(INT_M_CTLMASK,	INT_VECTOR_IRQ0);	 // Master, ICW2. IRQ0 -> 0x20.
	out_byte(INT_S_CTLMASK,	INT_VECTOR_IRQ8);	 // Slave , ICW2. IRQ8 -> 0x28
	out_byte(INT_M_CTLMASK,	0x4);			     // Master, ICW3. IR2 -> Slave 
	out_byte(INT_S_CTLMASK,	0x2);			     // Slave , ICW3. Slave -> Master IR2.
	out_byte(INT_M_CTLMASK,	0x1);			     // Master, ICW4.
	out_byte(INT_S_CTLMASK,	0x1);		         // Slave , ICW4.

	out_byte(INT_M_CTLMASK,	0xFE);	             // Master, OCW1. 
	out_byte(INT_S_CTLMASK,	0xFF);	             // Slave , OCW1. 
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

void init_idt_descriptor(unsigned char vector, uint8_t desc_type, pf_int_handler_t handler, unsigned char privilege){
    struct gate* p_gate	= &idt[vector];
	uint32_t	 base	= (uint32_t)handler;
	p_gate->offset_low	= base & 0xFFFF;
	p_gate->selector	= SELECTOR_KERNEL_CODE;
	p_gate->dcount		= 0;
	p_gate->attr		= desc_type | (privilege << 5);
	p_gate->offset_high	= (base >> 16) & 0xFFFF;
}

void init_idt(){
    init_8259a();

    // init interrupt gates (descriptors)
	init_idt_descriptor(INT_VECTOR_DIVIDE,	    DA_386IGate, divide_error,		    PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_DEBUG,		    DA_386IGate, single_step_exception,	PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_NMI,		    DA_386IGate, nmi,			        PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_BREAKPOINT,	DA_386IGate, breakpoint_exception,	PRIVILEGE_USER);
	init_idt_descriptor(INT_VECTOR_OVERFLOW,	    DA_386IGate, overflow,			    PRIVILEGE_USER);
	init_idt_descriptor(INT_VECTOR_BOUNDS,	    DA_386IGate, bounds_check,		    PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_INVAL_OP,	    DA_386IGate, inval_opcode,		    PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_COPROC_NOT,	DA_386IGate, copr_not_available,	PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_DOUBLE_FAULT,	DA_386IGate, double_fault,		    PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_COPROC_SEG,	DA_386IGate, copr_seg_overrun,		PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_INVAL_TSS,	    DA_386IGate, inval_tss,			    PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_SEG_NOT,	    DA_386IGate, segment_not_present,	PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_STACK_FAULT,	DA_386IGate, stack_exception,		PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_PROTECTION,	DA_386IGate, general_protection,	PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_PAGE_FAULT,	DA_386IGate, page_fault,		    PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_COPROC_ERR,	DA_386IGate, copr_error,		    PRIVILEGE_KRNL);

	init_idt_descriptor(INT_VECTOR_IRQ0 + 0,   	DA_386IGate, irq00,			    PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_IRQ0 + 1,  	DA_386IGate, irq01,			    PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_IRQ0 + 2,  	DA_386IGate, irq02,			    PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_IRQ0 + 3,  	DA_386IGate, irq03,			    PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_IRQ0 + 4,	    DA_386IGate, irq04,			    PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_IRQ0 + 5,	    DA_386IGate, irq05,			    PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_IRQ0 + 6,  	DA_386IGate, irq06,			    PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_IRQ0 + 7,  	DA_386IGate, irq07,			    PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_IRQ8 + 0,  	DA_386IGate, irq08,			    PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_IRQ8 + 1,	    DA_386IGate, irq09,			    PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_IRQ8 + 2,  	DA_386IGate, irq10,			    PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_IRQ8 + 3,  	DA_386IGate, irq11,			    PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_IRQ8 + 4,	    DA_386IGate, irq12,			    PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_IRQ8 + 5,  	DA_386IGate, irq13,			    PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_IRQ8 + 6,  	DA_386IGate, irq14,			    PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_IRQ8 + 7,  	DA_386IGate, irq15,			    PRIVILEGE_KRNL);
}

void exception_handler(int vec_no, int err_code, int eip, int cs, int eflags){   
    char err_description[][64] = {	
        "#DE Divide Error",
        "#DB RESERVED",
        "—  NMI Interrupt",
        "#BP Breakpoint",
        "#OF Overflow",
        "#BR BOUND Range Exceeded",
        "#UD Invalid Opcode (Undefined Opcode)",
        "#NM Device Not Available (No Math Coprocessor)",
        "#DF Double Fault",
        "    Coprocessor Segment Overrun (reserved)",
        "#TS Invalid TSS",
        "#NP Segment Not Present",
        "#SS Stack-Segment Fault",
        "#GP General Protection",
        "#PF Page Fault",
        "—  (Intel reserved. Do not use.)",
        "#MF x87 FPU Floating-Point Error (Math Fault)",
        "#AC Alignment Check",
        "#MC Machine Check",
        "#XF SIMD Floating-Point Exception"
    };

    kprint("Exception handler:");
    kprint(err_description[vec_no]);
    kprint("\n");
    kprint("EFLAGS: ");
    print_int_as_hex(eflags);
    kprint(" CS: ");
    print_int_as_hex(cs);
    kprint(" EIP: ");
    print_int_as_hex(eip);

    if(err_code != 0xffffffff){
        kprint("Error Code: ");
        print_int_as_hex(err_code);
    }    
}

void irq_handler(int irq){
    kprint("IRQ handler: ");
	print_int_as_hex(irq);
	kprint("\n");
}

void clock_handler(int irq){
	kprint("#");
	// round robin process scheduler.
	p_proc_ready++;
	if(p_proc_ready >= proc_table + MAX_TASKS_NUM)
		p_proc_ready = proc_table;
}