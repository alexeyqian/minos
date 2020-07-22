#include "interrupt.h"
#include "const.h"
#include "types.h"
#include "ktypes.h"
#include "global.h"
#include "klib.h"
#include "ke_asm_utils.h"
#include "screen.h" // kprint

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

#define	INT_VECTOR_SYSCALL	    	0x90


// 8259A interrupt controller ports
#define	INT_M_CTL	    0x20	/* I/O port for interrupt controller         <Master> */
#define	INT_M_CTLMASK	0x21	/* setting bits in this port disables ints   <Master> */
#define	INT_S_CTL	    0xA0	/* I/O port for second interrupt controller  <Slave>  */
#define	INT_S_CTLMASK	0xA1	/* setting bits in this port disables ints   <Slave>  */

// imported from asm
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


void irq_handler(int irq){
    kprint("IRQ handler: ");
	kprint_int_as_hex(irq);
}

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

	// disable all irq interrupts.
	out_byte(INT_M_CTLMASK,	0xFF);	             // Master, OCW1. 
	out_byte(INT_S_CTLMASK,	0xFF);	             // Slave , OCW1. 

	int i;
	for(i = 0; i < IRQ_NUM; i++)
		irq_table[i] = irq_handler;
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

	init_idt_descriptor(INT_VECTOR_SYSCALL,     DA_386IGate, syscall,               PRIVILEGE_USER);	

	uint16_t* p_idt_limit = (uint16_t*)(&idt_ptr[0]);
	uint32_t* p_idt_base  = (uint32_t*)(&idt_ptr[2]);
	*p_idt_limit = IDT_SIZE * sizeof(struct gate) - 1;
	*p_idt_base  = (uint32_t)&idt;

	load_idt();	
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
    kprint_int_as_hex(eflags);
    kprint(" CS: ");
    kprint_int_as_hex(cs);
    kprint(" EIP: ");
    kprint_int_as_hex(eip);

    if(err_code != 0xffffffff){
        kprint(" Error Code: ");
        kprint_int_as_hex(err_code);
    }    
}