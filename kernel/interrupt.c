#include "kernel.h"

// exceptions 
#define	INT_VECTOR_DIVIDE		    0x0
#define	INT_VECTOR_DEBUG		    0x1
#define	INT_VECTOR_NMI			    0x2
#define	INT_VECTOR_BREAKPOINT		0x3
#define	INT_VECTOR_OVERFLOW		    0x4
#define	INT_VECTOR_BOUNDS		    0x5
#define	INT_VECTOR_INVAL_OP		    0x6
#define	INT_VECTOR_COPROC_NOT		0x7
#define	INT_VECTOR_DOUBLE_FAULT		0x8
#define	INT_VECTOR_COPROC_SEG		0x9
#define	INT_VECTOR_INVAL_TSS		0xA
#define	INT_VECTOR_SEG_NOT		    0xB
#define	INT_VECTOR_STACK_FAULT		0xC
#define	INT_VECTOR_PROTECTION		0xD
#define	INT_VECTOR_PAGE_FAULT		0xE
#define	INT_VECTOR_COPROC_ERR		0x10

// Interrupt vector IRQ mapping
#define	INT_VECTOR_IRQ0			    0x20
#define	INT_VECTOR_IRQ8			    0x28

// MUST match var in ke_syscalls.inc
#define	INT_VECTOR_SYSCALL	    	0x90

// 8259A interrupt controller ports
#define	INT_M_CTL	    0x20	/* I/O port for interrupt controller         <Master> */
#define	INT_M_CTLMASK	0x21	/* setting bits in this port disables ints   <Master> */
#define	INT_S_CTL	    0xA0	/* I/O port for second interrupt controller  <Slave>  */
#define	INT_S_CTLMASK	0xA1	/* setting bits in this port disables ints   <Slave>  */

// used for clock_handler to wake up TASK_TTY when a key is pressed 
PRIVATE int key_pressed = 0; 

PUBLIC pf_irq_handler_t irq_table[NR_IRQ];

PUBLIC void set_key_pressed(int value){
	key_pressed = value;
}

PUBLIC void irq_handler(int irq){
    kprintf("IRQ handler: 0x%x\n", irq);
}

PRIVATE void clock_irq_handler(int irq){
	UNUSED(irq);
	
	if(++ticks >= MAX_TICKS) ticks = 0;

	if(p_proc_ready->ticks)
		p_proc_ready->ticks--;

	if(key_pressed) 
		inform_int(TASK_TTY);
		
	if(k_reenter != 0){ // interrupt re-enter
		return;
	}

	if (p_proc_ready->ticks > 0) return;
	schedule(); 
}

#define REG_STATUS	0x1F7 // TODO: duplicated define in _hd.h
PRIVATE void hd_irq_handler()
{
	// interrupts are cleared when the host
	// reads the status register
	// issues a reset, or
	// writes to the command register
	in_byte(REG_STATUS); // read out the status so the hd contruller can have interrupt again.
	inform_int(TASK_HD);
}

PRIVATE KB_INPUT kb_in;

// TODO: move, should only be used by keyboard driver
PUBLIC uint8_t read_from_kb_buf()	
{
	uint8_t	scan_code;
	while (kb_in.count <= 0) {} // waiting for at least one scan code

	clear_intr(); 
	scan_code = (uint8_t)(*(kb_in.p_tail));
	kb_in.p_tail++;
	if (kb_in.p_tail == kb_in.buf + KB_IN_BYTES) {
		kb_in.p_tail = kb_in.buf;
	}
	kb_in.count--;
	set_intr();

	return scan_code;
}

/*
	// for 1 byte  scan code, each keypress/release fires 2 interrupts (1 make code and 1 break code)
	// for 2 bytes scan code, each keypress/release fires 4 interrupts (2 make codes and 2 break codes, start with E0)
	// for 3 bytes scan code, PUASE, only has make code, no break code, so only fires 3 interrupts (start with E1)
	// one for make code (press), one for break code (release).
	// scan code has 2 types: make code and break code
*/
// Called per interrupt
PRIVATE void keyboard_irq_handler(int irq){
	UNUSED(irq);
	
	//append scan code to kb buf
	uint8_t scan_code = in_byte(KB_DATA);   // read out from 8042, so it can response for next key interrupt.
	clear_intr();
	if(kb_in.count >= KB_IN_BYTES) return;  // ignre scan code if buffer is full
	
	*(kb_in.p_head) = (char)scan_code; // safe convert here
	kb_in.p_head++;

	if(kb_in.p_head == kb_in.buf + KB_IN_BYTES)
		kb_in.p_head = kb_in.buf;
	
	kb_in.count++;	
	set_intr();
	//kprintf("key:[%x] ", scan_code);
	key_pressed = 1; 

}

PRIVATE void init_kb_handler_data(){
	// init keyboard buf
	kb_in.count = 0;
	kb_in.p_head = kb_in.p_tail = kb_in.buf;
}

PRIVATE void put_irq_handler(int irq, pf_irq_handler_t handler){
	disable_irq(irq);
	irq_table[irq] = handler;
}

// setup chip 8259A which is a bridge between interrupting devices and CPU
// ICW: Initialization Command Word
// there are 4 types of ICW, from ICW1 - ICW4, each has specific format
// The write operation must be ordered correctly
// In real mode, BIOS map IRQ0-IRQ7 TO 0X8 - 0XF, but in proteced mode these are occupied,
// so we need to remap them to 0x20 and 0x28
PRIVATE void init_8259a(){
    out_byte(INT_M_CTL,	0x11);			         // Master, ICW1.
	out_byte(INT_S_CTL,	0x11);			         // Slave , ICW1.
	out_byte(INT_M_CTLMASK,	INT_VECTOR_IRQ0);	 // Master, ICW2. IRQ0 -> 0x20.
	out_byte(INT_S_CTLMASK,	INT_VECTOR_IRQ8);	 // Slave , ICW2. IRQ8 -> 0x28
	out_byte(INT_M_CTLMASK,	0x4);			     // Master, ICW3. IR2 -> Slave 
	out_byte(INT_S_CTLMASK,	0x2);			     // Slave , ICW3. Slave -> Master IR2.
	out_byte(INT_M_CTLMASK,	0x1);			     // Master, ICW4.
	out_byte(INT_S_CTLMASK,	0x1);		         // Slave , ICW4.

	// disable all irq interrupts.
	out_byte(INT_M_CTLMASK,	0xFF);	             // Master, OCW1. 
	out_byte(INT_S_CTLMASK,	0xFF);	             // Slave , OCW1. 

	for(int i = 0; i < NR_IRQ; i++)
		irq_table[i] = irq_handler;

	// setup special handlers
	put_irq_handler(CLOCK_IRQ, clock_irq_handler);
	init_kb_handler_data();
	put_irq_handler(KEYBOARD_IRQ, keyboard_irq_handler);
	put_irq_handler(AT_WINI_IRQ, hd_irq_handler);	
}

PRIVATE void init_idt_descriptor(unsigned char vector, uint8_t desc_type, pf_int_handler_t handler, unsigned char privilege){
    struct gate* p_gate	= &idt[vector];
	uint32_t	 base	= (uint32_t)handler;
	p_gate->offset_low	= (uint16_t)(base & 0xFFFF);
	p_gate->selector	= SELECTOR_KERNEL_CODE;
	p_gate->dcount		= 0;
	p_gate->attr		= desc_type | (uint8_t)(privilege << 5);
	p_gate->offset_high	= (uint16_t)((base >> 16) & 0xFFFF);
}

PUBLIC void init_idt(){
	init_8259a();
	
    // init exception gates (descriptors)
	init_idt_descriptor(INT_VECTOR_DIVIDE,	    DA_386IGate, divide_error,		    PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_DEBUG,		DA_386IGate, single_step_exception,	PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_NMI,		    DA_386IGate, nmi,			        PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_BREAKPOINT,	DA_386IGate, breakpoint_exception,	PRIVILEGE_USER);
	init_idt_descriptor(INT_VECTOR_OVERFLOW,	DA_386IGate, overflow,			    PRIVILEGE_USER);
	init_idt_descriptor(INT_VECTOR_BOUNDS,	    DA_386IGate, bounds_check,		    PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_INVAL_OP,	DA_386IGate, inval_opcode,		    PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_COPROC_NOT,	DA_386IGate, copr_not_available,	PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_DOUBLE_FAULT,DA_386IGate, double_fault,		    PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_COPROC_SEG,	DA_386IGate, copr_seg_overrun,		PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_INVAL_TSS,	DA_386IGate, inval_tss,			    PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_SEG_NOT,	    DA_386IGate, segment_not_present,	PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_STACK_FAULT,	DA_386IGate, stack_exception,		PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_PROTECTION,	DA_386IGate, general_protection,	PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_PAGE_FAULT,	DA_386IGate, page_fault,		    PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_COPROC_ERR,	DA_386IGate, copr_error,		    PRIVILEGE_KRNL);
	// init interrupt gates (descriptors)
	init_idt_descriptor(INT_VECTOR_IRQ0 + 0,   	DA_386IGate, irq00,			        PRIVILEGE_KRNL); // 0x20
	init_idt_descriptor(INT_VECTOR_IRQ0 + 1,  	DA_386IGate, irq01,			        PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_IRQ0 + 2,  	DA_386IGate, irq02,			        PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_IRQ0 + 3,  	DA_386IGate, irq03,			        PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_IRQ0 + 4,	DA_386IGate, irq04,			        PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_IRQ0 + 5,	DA_386IGate, irq05,			        PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_IRQ0 + 6,  	DA_386IGate, irq06,			        PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_IRQ0 + 7,  	DA_386IGate, irq07,			        PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_IRQ8 + 0,  	DA_386IGate, irq08,			        PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_IRQ8 + 1,	DA_386IGate, irq09,			        PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_IRQ8 + 2,  	DA_386IGate, irq10,			        PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_IRQ8 + 3,  	DA_386IGate, irq11,			        PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_IRQ8 + 4,	DA_386IGate, irq12,			        PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_IRQ8 + 5,  	DA_386IGate, irq13,			        PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_IRQ8 + 6,  	DA_386IGate, irq14,			        PRIVILEGE_KRNL);
	init_idt_descriptor(INT_VECTOR_IRQ8 + 7,  	DA_386IGate, irq15,			        PRIVILEGE_KRNL);
	// init system call
	init_idt_descriptor(INT_VECTOR_SYSCALL,     DA_386IGate, syscall,               PRIVILEGE_USER);	

	uint16_t* p_idt_limit = (uint16_t*)(&idt_ptr[0]);
	uint32_t* p_idt_base  = (uint32_t*)(&idt_ptr[2]);
	*p_idt_limit = IDT_SIZE * sizeof(struct gate) - 1;
	*p_idt_base  = (uint32_t)&idt;

	load_idt();	// TODO: use &idt_ptr to avoid import var to asm
}

PUBLIC void exception_handler(int vec_no, int err_code, int eip, int cs, int eflags){   
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

    kprintf("Exception handler: %d\n", err_description[vec_no]);
    kprintf("EFLAGS: 0x%x, CS: 0x%x, EIP: 0x%x\n", eflags, cs, eip);

    if(err_code != (int)0xffffffff)
        kprintf(" Error Code: 0x%x\n", err_code);    
}
