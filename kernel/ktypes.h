#ifndef _MINOS_KTYPES_H_
#define _MINOS_KTYPES_H_

#include "types.h"
#include "const.h"
#include "../include/minos/type.h"


typedef int proc_nr_t; // process table entry number
typedef short sys_id_t; // system process index

struct memory{
	phys_clicks base;
	phys_clicks size;
};

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


struct mess1{
    int m1i1;
    int m1i2;
    int m1i3;
    int m1i4;
};

struct mess2{
    void* m2p1;
    void* m2p2;
    void* m2p3;
    void* m2p4;
};

struct mess3{
    int m3i1;
    int m3i2;
    int m3i3;
    int m3i4;
    uint64_t m3l1;
    uint64_t m3l2;
    void* m3p1;
    void* m3p2;
};

typedef struct{
    int source;
    int type;
    union{
        struct mess1 m1;
        struct mess2 m2;
        struct mess3 m3;
    }u;
}MESSAGE;

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
// which in turn points to the ldt in struct proc
// this struct is called  PCB in OS books
typedef struct proc{
    struct stack_frame  regs;           // process registers saved in stack frame
    uint16_t            ldt_sel;        // selector in gdt giving the ldt base and limit
    // TODO: rename to process_ldt
	struct descriptor   ldt[LDT_SIZE]; // local descriptors for code and data
                                        // descriptor 1 for code, descriptor 2 for data
    uint32_t            pid;
    char                p_name[16];               // process name
	int                 ticks;
	int                 priority;

	int p_flags; // runnable if = 0
	MESSAGE* p_msg;
	int p_recvfrom; // pid_t: indicates from who this proc wants to receive msg?
	int p_sendto;   // pid_t
	int has_int_msg; // non zero if an INTERRUPT occurred when the task is not ready to deal with it.
	struct proc* q_sending; //queue of procs sending message to this proc
	struct proc* next_sending; // next proc in the sending queue
	int 				tty_idx;
}proc_s;

// The paging has made LDT almost obsolete, and there is no longer need for multiple LDT descriptors.

typedef struct task{
	pf_task_t initial_eip;
	int stack_size;
	char name[32];
}task_s;


#endif