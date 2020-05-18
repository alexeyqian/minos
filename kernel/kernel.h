#ifndef _MINOS_KERNEL_H_
#define _MINOS_KERNEL_H_

#include "types.h"

#define	PRIVILEGE_KRNL	0
#define	PRIVILEGE_TASK	1
#define	PRIVILEGE_USER	3

#define RPL_KRNL SA_RPL0
#define RPL_TASK SA_RPL1
#define RPL_USER SA_RPL3    

#define GDT_SIZE 128
#define IDT_SIZE 256
// GDT segments index
#define	INDEX_NULL		    0	// ┓
#define	INDEX_CODE		    1	// ┣ Defined in loader
#define	INDEX_DATA	    	2	// ┃
#define	INDEX_VIDEO		    3	// ┛
#define INDEX_TSS           4
#define INDEX_LDT_FIRST     5

// GDT selectors
#define	SELECTOR_NULL		0    		// ┓
#define	SELECTOR_CODE	    0x08		// ┣ Fixed in loader
#define	SELECTOR_DATA   	0x10		// ┃
#define	SELECTOR_VIDEO	    (0x18+3)    // ┛<-- RPL=3
#define SELECTOR_TSS        0x20
#define SELECTOR_LDT_FIRST  0x28

#define	SELECTOR_KERNEL_CODE	SELECTOR_CODE
#define	SELECTOR_KERNEL_DATA	SELECTOR_DATA
#define SELECTOR_KERNEL_VIDEO   SELECTOR_VIDEO

// each task has it's own LDT, each LDT contains 2 descriptors
#define LDT_SIZE             2

// SA: Selector Attribute
#define SA_RPL_MASK    0xfffc
#define SA_RPL0        0
#define SA_RPL1        1
#define SA_RPL2        2
#define SA_RPL3        3
#define SA_TI_MASK     0xfffb
#define SA_TIG         0
#define SA_TIL         4

// Descriptor Attributes
#define	DA_32			0x4000	/* 32 位段				*/
#define	DA_LIMIT_4K		0x8000	/* 段界限粒度为 4K 字节			*/
#define	DA_DPL0			0x00	/* DPL = 0				*/
#define	DA_DPL1			0x20	/* DPL = 1				*/
#define	DA_DPL2			0x40	/* DPL = 2				*/
#define	DA_DPL3			0x60	/* DPL = 3				*/
/* 存储段描述符类型值说明 */
#define	DA_DR			0x90	/* 存在的只读数据段类型值		*/
#define	DA_DRW			0x92	/* 存在的可读写数据段属性值		*/
#define	DA_DRWA			0x93	/* 存在的已访问可读写数据段类型值	*/
#define	DA_C			0x98	/* 存在的只执行代码段属性值		*/
#define	DA_CR			0x9A	/* 存在的可执行可读代码段属性值		*/
#define	DA_CCO			0x9C	/* 存在的只执行一致代码段属性值		*/
#define	DA_CCOR			0x9E	/* 存在的可执行可读一致代码段属性值	*/
/* 系统段描述符类型值说明 */
#define	DA_LDT			0x82	/* 局部描述符表段类型值			*/
#define	DA_TaskGate		0x85	/* 任务门类型值				*/
#define	DA_386TSS		0x89	/* 可用 386 任务状态段类型值		*/
#define	DA_386CGate		0x8C	/* 386 调用门类型值			*/
#define	DA_386IGate		0x8E	/* 386 中断门类型值			*/
#define	DA_386TGate		0x8F	/* 386 陷阱门类型值			*/

// 8259A interrupt controller ports. 
#define	INT_M_CTL	0x20	    /* I/O port for interrupt controller         <Master> */
#define	INT_M_CTLMASK	0x21	/* setting bits in this port disables ints   <Master> */
#define	INT_S_CTL	0xA0	    /* I/O port for second interrupt controller  <Slave>  */
#define	INT_S_CTLMASK	0xA1	/* setting bits in this port disables ints   <Slave>  */

// Interrupt Vector
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

#define MAX_TASKS_NUM 3  
#define STACK_SIZE_TESTA 0x8000
#define STACK_SIZE_TESTB 0x8000
#define STACK_SIZE_TESTC 0x8000
#define STACK_SIZE_TOTAL STACK_SIZE_TESTA+STACK_SIZE_TESTB+STACK_SIZE_TESTC

// MACRO: linear address to physical address
#define virtual_to_physical(seg_base, virtual) (uint32_t)(((uint32_t)seg_base) + (uint32_t)(virtual))

typedef struct descriptor{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t attr1;
    uint8_t limit_high_attr2;
    uint8_t base_high;
}descriptor_s;

typedef struct gate{
    uint16_t offset_low;
    uint16_t selector;    
    // only used in call gate, number of parameters 
    // which need to be copied to kernel stack
    uint8_t  dcount;      
    uint8_t  attr;        // P(1)DPL(2)DT(1)TYPE(4)
    uint16_t offset_high;
} gate_s;

// tss has a descriptor in GDT, base is the address of tss, limit is the size of the tss.
// inside tss, ss0 should be set to kernel data segment descriptor.
// esp0 gets the value the stack-pointer shall get at a system call.
// iopb may get the value of sizeof(TSS) which is 104, if you don't plan to use this io-bitmap further
// we need TSS per processor, and most of time use esp0, other tss contents are irrelevant.
// tss may reside anywhere in memory, a segment register call task register (TR) holds a segment selector
// that points to a valid TSS segment descriptor which resides in GDT. (a TSS descriptor may not reside in the LDT)
// Therefore, to use a TSS the following must be done by the operation system kernel:
// 1. create a tss descriptor entry in the GDT
// 2. load the TR with the segment selector for that segment
// 3. add information to the TSS in memory as needed.

// modern OS such as linux and windows donot use TSS fields (hardware task switch), as they implement software task switching.
// linux creates only one TSS for each CPU, and uses them for all tasks. This approach was selected as it provides easier portability
// to other architectures, for ex, amd64 does not support hardware task switches.
// linux only use the io port permission bitmap and inner stack features of the tss;
// the other reatures are only needed for hardware task switches, whcih the linux kernel does not use.
typedef struct tss{
    uint32_t	backlink;
	uint32_t	esp0;		// stack pointer to use during interrupt
	uint32_t	ss0;		
	uint32_t	esp1;
	uint32_t	ss1;
	uint32_t	esp2;
	uint32_t	ss2;
	uint32_t	cr3;
	uint32_t	eip;
	uint32_t	flags;
	uint32_t	eax;
	uint32_t	ecx;
	uint32_t	edx;
	uint32_t	ebx;
	uint32_t	esp;
	uint32_t	ebp;
	uint32_t	esi;
	uint32_t	edi;
	uint32_t	es;
	uint32_t	cs;
	uint32_t	ss;
	uint32_t	ds;
	uint32_t	fs;
	uint32_t	gs;
	uint32_t	ldt;
	uint16_t	trap;
	uint16_t	iobase;	// if I/O base >= TSS limit，means no IOPL map // TODO: renamed to iopb_offset
}tss_s;

typedef struct stack_frame{ // proc_ptr points to here
    uint32_t	gs;		    /* ┓						│Low address*/ 
	uint32_t	fs;		    /* ┃						│			*/
	uint32_t	es;		    /* ┃						│			*/
	uint32_t	ds;		    /* ┃						│			*/
	uint32_t	edi;		/* ┃						│			*/
	uint32_t	esi;		/* ┣ pushed by save()		│			*/
	uint32_t	ebp;		/* ┃						│			*/
	uint32_t	kernel_esp;	/* <- 'popad' will ignore it│ TODO: why??		*/
	uint32_t	ebx;		/* ┃						↑Stack from high to low*/		
	uint32_t	edx;		/* ┃						│			*/
	uint32_t	ecx;		/* ┃						│			*/
	uint32_t	eax;		/* ┛						│			*/
	uint32_t	retaddr;	/* return address for assembly code save()	│ */
	uint32_t	eip;		/*  ┓						│			*/
	uint32_t	cs;		    /*  ┃						│			*/
	uint32_t	eflags;  	/*  ┣ these are pushed by CPU during interrupt	│ */
	uint32_t	esp;     	/*  ┃						│			*/
	uint32_t	ss;         /*  ┛   	                |High Address */
}stack_frame_s;

// here is how it looks like in memory
// left: low addr, right: high addr
// stack_frame(gs, fs, es, ds, edi, esi, ebp, esp, ebx, edx, ecx eax)
// -> retaddr, eip, cs, eflags, esp, ss,
// -> ldt selector (points to a descriptor in GDT), descriptor 1, descriptor 2.

// in GDT: ... video descroptor ... 
// TSS descriptor (points to TSS)
// -> LDT descriptor(points to desc1 and desc2) ...

// ldt is part of process
// ldt_sel points to the LDT descriptor in GDT, 
// which in turn points to the ldts in struct proc
typedef struct proc{
    struct stack_frame  regs;           // process registers saved in stack frame
    uint16_t            ldt_sel;        // selector in gdt giving the ldt base and limit
    // TODO: rename to process_ldt
	struct descriptor   ldts[LDT_SIZE]; // local descriptors for code and data
                                        // descriptor 1 for code, descriptor 2 for data
    uint32_t            pid;
    char                p_name[16];               // process name
}proc_s;

// The paging has made LDT almost obsolete, and there is no longer need for multiple LDT descriptors.

typedef struct task{
	pf_task_t initial_eip;
	int stack_size;
	char name[32];
}task_s;
#endif