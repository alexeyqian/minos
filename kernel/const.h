#ifndef _MINOS_CONST_H_
#define _MINOS_CONST_H_

#include "../include/minos/type.h"

#define	PRIVILEGE_KRNL	0
#define	PRIVILEGE_TASK	1
#define	PRIVILEGE_USER	3

#define RPL_KRNL SA_RPL0
#define RPL_TASK SA_RPL1
#define RPL_USER SA_RPL3    

#define TASKS_NUM 1
#define PROCS_NUM 3 
#define STACK_SIZE_TTY   0x8000
#define STACK_SIZE_TESTA 0x8000
#define STACK_SIZE_TESTB 0x8000
#define STACK_SIZE_TESTC 0x8000
#define STACK_SIZE_TOTAL STACK_SIZE_TTY+STACK_SIZE_TESTA+STACK_SIZE_TESTB+STACK_SIZE_TESTC

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

#define SYSCALLS_NUM  2

#define TIMER0          0x40
#define TIMER_MODE      0x43
#define RATE_GENERATOR  0x34
#define TIMER_FREQ      1193182L
#define HZ              100

#define IRQ_NUM         16

// hardware interrupts
#define	CLOCK_IRQ	    0
#define	KEYBOARD_IRQ	1 // TODO: move to keyboard.h
#define	CASCADE_IRQ   	2	/* cascade enable for 2nd AT controller */
#define	ETHER_IRQ	    3	/* default ethernet interrupt vector */
#define	SECONDARY_IRQ	3	/* RS232 interrupt vector for port 2 */
#define	RS232_IRQ	    4	/* RS232 interrupt vector for port 1 */
#define	XT_WINI_IRQ	    5	/* xt winchester */
#define	FLOPPY_IRQ   	6	/* floppy disk */
#define	PRINTER_IRQ	    7
#define	AT_WINI_IRQ	   14	/* at winchester */

/* Color */
/*
 * e.g.	MAKE_COLOR(BLUE, RED)
 *	MAKE_COLOR(BLACK, RED) | BRIGHT
 *	MAKE_COLOR(BLACK, RED) | BRIGHT | FLASH
 */
#define	BLACK	0x0 	/* 0000 */
#define	WHITE	0x7 	/* 0111 */
#define	RED	    0x4 	/* 0100 */
#define	GREEN	0x2 	/* 0010 */
#define	BLUE	0x1 	/* 0001 */
#define	FLASH	0x80	/* 1000 0000 */
#define	BRIGHT	0x08	/* 0000 1000 */
#define	MAKE_COLOR(x,y)	((x<<4) | y)	/* MAKE_COLOR(Background,Foreground) */

// AT keyboard: 8042 ports 
#define	KB_DATA		0x60	//I/O port for keyboard data
					        //Read : Read Output Buffer 
					        //Write: Write Input Buffer(8042 Data&8048 Command)
#define	KB_CMD		0x64	// I/O port for keyboard command
					        //Read : Read Status Register
					        //Write: Write Input Buffer(8042 Command)

// VGA
#define CRTC_ADDR_REG			    0x3D4	/* CRT Controller Registers - Address Register */
#define CRTC_DATA_REG			    0x3D5	/* CRT Controller Registers - Data Registers */
#define CRTC_DATA_IDX_START_ADDR_H	0xC	    /* register index of video mem start address (MSB) */
#define CRTC_DATA_IDX_START_ADDR_L	0xD  	/* register index of video mem start address (LSB) */
#define CRTC_DATA_IDX_CURSOR_H		0xE  	/* register index of cursor position (MSB) */
#define CRTC_DATA_IDX_CURSOR_L		0xF  	/* register index of cursor position (LSB) */
#define V_MEM_BASE			       0xB8000	/* base of color video memory */
#define V_MEM_SIZE			       0x8000	/* 32K: B8000H -> BFFFFH */

#define NR_CONSOLES 3

/* Sizes of memory tables. The boot monitor distinguishes three memory areas, 
 * namely low mem below 1M, 1M-16M, and mem after 16M. More chunks are needed
 * for MINOS.
 */
#define NR_MEMS            8	

#endif