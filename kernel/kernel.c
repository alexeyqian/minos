
#include "const.h"
#include "types.h"
#include "klib.h"

//global variables
int			        disp_pos;
uint8_t			    gdt_ptr[6];	// 0~15:Limit  16~47:Base
struct descriptor   gdt[GDT_SIZE];
uint8_t			    idt_ptr[6];	// 0~15:Limit  16~47:Base
struct gate			idt[IDT_SIZE];


void spurious_irq(int irq);

void kstart(){
    kprint_str("\n kstart ...");

    // copy old gdt_ptr to new gdt
    kmemcpy(&gdt, // new gdt
        (void*)(*((uint32_t*)(&gdt_ptr[2]))),   // Base  of Old GDT
		*((uint16_t*)(&gdt_ptr[0])) + 1	    // Limit of Old GDT
	);

    uint16_t* p_gdt_limit = (uint16_t*)(&gdt_ptr[0]);
	uint32_t* p_gdt_base  = (uint32_t*)(&gdt_ptr[2]);
	*p_gdt_limit = GDT_SIZE * sizeof(struct descriptor) - 1;
	*p_gdt_base  = (uint32_t)&gdt;
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

	out_byte(INT_M_CTLMASK,	0xFD);	             // Master, OCW1. 
	out_byte(INT_S_CTLMASK,	0xFF);	             // Slave , OCW1. 
}

void spurious_irq(int irq){
    kprint_str("spurious_irq: ");
	//kprint_int_as_hex(irq);
	kprint_str("\n");
}

void exception_handler(int vec_no, int err_code, int eip, int cs, int eflags){
    int i;
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


}