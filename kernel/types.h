#ifndef _MINOS_TYPES_H_
#define _MINOS_TYPES_H_

#define TRUE  1
#define FALSE 0

typedef int              bool_t;
typedef char             int8_t;
typedef unsigned char    uint8_t;
typedef short            int16_t;
typedef unsigned short   uint16_t;
typedef int              int32_t;
typedef unsigned int     uint32_t;

typedef int32_t          intptr_t;
typedef uint32_t         uintptr_t;

typedef uint32_t         size_t;
typedef int32_t          ssize_t;
typedef int32_t          off_t;

// TODO: below should be moved to kernel_types.h
typedef unsigned short io_port_t;
typedef void (*pf_int_handler_t)();

struct descriptor{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t attr;
    uint8_t limit_high_attr2;
    uint8_t base_high;
};

struct gate{
    uint16_t offset_low;
    uint16_t selector;    
    // only used in call gate, number of parameters 
    // which need to be copied to kernel stack
    uint8_t  dcount;      
    uint8_t  attr;        // P(1)DPL(2)DT(1)TYPE(4)
    uint16_t offset_high;
};

struct stack_frame{ // proc_ptr points to here
    uint32_t	gs;		    /* ┓						│Low address*/ 
	uint32_t	fs;		    /* ┃						│			*/
	uint32_t	es;		    /* ┃						│			*/
	uint32_t	ds;		    /* ┃						│			*/
	uint32_t	edi;		/* ┃						│			*/
	uint32_t	esi;		/* ┣ pushed by save()		│			*/
	uint32_t	ebp;		/* ┃						│			*/
	uint32_t	kernel_esp;	/* <- 'popad' will ignore it│			*/
	uint32_t	ebx;		/* ┃						↑Stack from high to low*/		
	uint32_t	edx;		/* ┃						│			*/
	uint32_t	ecx;		/* ┃						│			*/
	uint32_t	eax;		/* ┛						│			*/
	uint32_t	retaddr;	/* return address for assembly code save()	│			*/
	uint32_t	eip;		/*  ┓						│			*/
	uint32_t	cs;		    /*  ┃						│			*/
	uint32_t	eflags;  	/*  ┣ these are pushed by CPU during interrupt	│			*/
	uint32_t	esp;     	/*  ┃						│			*/
	uint32_t	ss;         /*  ┛   	                |High Address */
};

struct proc{
    struct stack_frame  regs; // process registers saved in stack frame
    uint16_t            ldt_sel;   // selector in gdt giving the ldt base and limit
    struct descriptor   ldts[LDT_SIZE]; // local descriptors for code and data
                                      // LDT_SIZE = 2
    uint32_t            pid;
    char                p_name[16];               // process name
};

#define NR_TASKS 1 // number of tasks
#define STACK_SIZE_TESTA 0x8000
#define STCK_SIZE_TOTAL STACK_SIZE_TESTSA

#endif