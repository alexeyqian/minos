#ifndef MINOS_CONST_H
#define MINOS_CONST_H

// ============ common ====================
#define EXTERN  extern
#define PRIVATE static
#define PUBLIC 
#define FORWARD static

#define TRUE  1
#define FALSE 0
#define true TRUE
#define false FALSE

#define NULL ((void *)0)
// use expression as sub-expression,
// then make type of full expression int, discard result
#define UNUSED(x) (void)(x) // (void)(sizeof((x), 0))

// ============= GDT/LDT/IDT =======================
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
#define LDT_SIZE                2
#define INDEX_LDT_C             0
#define INDEX_LDT_RW            1

#define	PRIVILEGE_KRNL	0
#define	PRIVILEGE_TASK	1
#define	PRIVILEGE_USER	3

#define RPL_KRNL SA_RPL0
#define RPL_TASK SA_RPL1
#define RPL_USER SA_RPL3    

// SA: Selector Attribute
#define SA_RPL_MASK    0xfffc // C=0011b
#define SA_RPL0        0
#define SA_RPL1        1
#define SA_RPL2        2
#define SA_RPL3        3
#define SA_TI_MASK     0xfffb // B=1011b
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

#define	LIMIT_4K_SHIFT 12

// ======= CLOCK ================
#define TIMER0          0x40
#define TIMER_MODE      0x43
#define RATE_GENERATOR  0x34
#define TIMER_FREQ      1193182L
#define HZ              100

#define CLICK_SIZE      1024 // unit in which memory is allocated
#define CLICK_SHIFT     10 // log2 of CLICK_SIZE

// ========= IRQ ==============
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


//  =================== AT keyboard: 8042 ports ==============
#define	KB_DATA		0x60	//I/O port for keyboard data
					        //Read : Read Output Buffer 
					        //Write: Write Input Buffer(8042 Data&8048 Command)
#define	KB_CMD		0x64	// I/O port for keyboard command
					        //Read : Read Status Register
					        //Write: Write Input Buffer(8042 Command)

// ============= VGA ==================
#define NR_CONSOLES                3

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

//#define DEFAULT_CHAR_COLOR	 0x07	/* 0000 0111 黑底白字 */
#define DEFAULT_CHAR_COLOR	(MAKE_COLOR(BLACK, WHITE))
#define GRAY_CHAR		(MAKE_COLOR(BLACK, BLACK) | BRIGHT)
#define RED_CHAR		(MAKE_COLOR(BLUE, RED) | BRIGHT)

// ========= TASKS ==================
#define NR_TASKS        6
#define NR_PROCS        32
#define NR_NATIVE_PROCS 4
#define NR_SYSCALLS     2

#define PROCS_BASE 0xa00000 // 10MB
#define PROC_IMAGE_SIZE_DEFAULT 0x100000 // 1MB
#define PROC_ORIGIN_STACK 0x400 // 1KB

// task types should match global vars: task_table
#define INVALID_DRIVER	-20
#define INTERRUPT	    -10

#define TASK_SYS	    0 // system call
#define TASK_HD		    1 // hardisk driver, depend on sys
#define TASK_FS	        2 // file system, depend on hd
#define TASK_TTY	    3 // terminal, depend on fs
#define TASK_MM         4 // memory management
#define TASK_TEST       5

#define INIT            6 // first user proc
#define ANY		       (NR_TASKS + NR_PROCS + 10)
#define NO_TASK		   (NR_TASKS + NR_PROCS + 20)

#define	MAX_TICKS	0x7FFFABCD

#define STACK_SIZE_TTY   0x8000  
#define STACK_SIZE_SYS   0x8000
#define STACK_SIZE_HD    0x8000
#define STACK_SIZE_FS    0x8000
#define STACK_SIZE_MM    0x8000
#define STACK_SIZE_TEST  0x8000

#define STACK_SIZE_INIT  0x8000
#define STACK_SIZE_TESTA 0x8000
#define STACK_SIZE_TESTB 0x8000
#define STACK_SIZE_TESTC 0x8000
#define STACK_SIZE_TOTAL STACK_SIZE_SYS+STACK_SIZE_HD \
	+STACK_SIZE_FS+STACK_SIZE_TTY+STACK_SIZE_MM+STACK_SIZE_TEST \
	+STACK_SIZE_INIT+STACK_SIZE_TESTA+STACK_SIZE_TESTB+STACK_SIZE_TESTC

// ========= memory ===================
/* Sizes of memory tables. The boot monitor distinguishes three memory areas, 
 * namely low mem below 1M, 1M-16M, and mem after 16M. More chunks are needed
 * for MINOS.
 */
#define NR_MEMS            8	

// Memory management
#define SEGMENT_TYPE  0xFF00
#define SEGMENT_INDEX 0x00FF

#define LOCAL_SEG 0x0000
#define NR_LOCAL_SEGS  3 // local segments per process (fixed)
#define T              0 // proc[i].mem_map[T] is for text
#define D              1 // for data
#define S              2 // for stack

#define REMOTE_SEG   0x0100
#define NR_REMOTE_SEGS    3

#define BIOS_SEG     0x0200
#define NR_BIOS_SEGS      3

#define PHYS_SEG     0x0400

// magic chars used by 'printx'
#define MAG_CH_PANIC   '\002'
#define MAG_CH_ASSERT  '\003'

// ======== HD related constants ==================
#define	DIOCTL_GET_GEO	1

/* Hard Drive */
#define SECTOR_SIZE		512
#define SECTOR_BITS		(SECTOR_SIZE * 8)
#define SECTOR_SIZE_SHIFT	9

/* major device numbers (corresponding to kernel/global.c::dd map array) */
#define	NO_DEV			0
#define	DEV_FLOPPY		1
#define	DEV_CDROM		2
#define	DEV_HD			3
#define	DEV_CHAR_TTY    4
#define	DEV_SCSI		5
/* make device number from major and minor numbers */
#define	MAJOR_SHIFT		8
#define	MAKE_DEV(a,b)		((a << MAJOR_SHIFT) | b)
/* separate major and minor numbers from device number */
#define	MAJOR(x)		((x >> MAJOR_SHIFT) & 0xFF)
#define	MINOR(x)		(x & 0xFF)

#define	INVALID_INODE		0
#define	ROOT_INODE          1

#define	MAX_DRIVES          2
#define	NR_PART_PER_DRIVE	4
#define	NR_SUB_PER_PART		16
#define	NR_SUB_PER_DRIVE	(NR_SUB_PER_PART * NR_PART_PER_DRIVE)
#define	NR_PRIM_PER_DRIVE	(NR_PART_PER_DRIVE + 1)

/**
 * @def MAX_PRIM_DEV
 * Defines the max minor number of the primary partitions.
 * If there are 2 disks, prim_dev ranges in hd[0-9], this macro will
 * equals 9.
 */
#define	MAX_PRIM		(MAX_DRIVES * NR_PRIM_PER_DRIVE - 1)

#define	MAX_SUBPARTITIONS	(NR_SUB_PER_DRIVE * MAX_DRIVES)

/* device numbers of hard disk */
#define	MINOR_hd1a		0x10
#define MINOR_hd2a      (MINOR_hd1a + NR_SUB_PER_PART)
//#define	MINOR_hd2a		0x20
//#define	MINOR_hd2b		0x21
//#define	MINOR_hd3a		0x30
//#define	MINOR_hd4a		0x40

#define MINOR_BOOT MINOR_hd2a // TODO: move to config.h
#define	ROOT_DEV		MAKE_DEV(DEV_HD, MINOR_BOOT)	/* 3, 0x21 */

#define	P_PRIMARY	0
#define	P_EXTENDED	1

#define ORANGES_PART	0x99	/* Orange'S partition */
#define NO_PART	    	0x00	/* unused entry */
#define EXT_PART    	0x05	/* extended partition */

// =============== file system =================
#define	NR_FILES	    64 // max files a proc can open at same time
#define	NR_FILE_DESC	64	/* FIXME */
#define	NR_INODE	    64	/* FIXME */
#define	NR_SUPER_BLOCK	8

#define	INSTALL_START_SECT		0x8000
#define	INSTALL_NR_SECTS		0x800

// =================== IPC messages ==============
#define	FD		    u.m3.m3i1 
#define	PATHNAME	u.m3.m3p1 
#define	FLAGS		u.m3.m3i1 
#define	NAME_LEN	u.m3.m3i2 
#define BUF_LEN     u.m3.m3i3
#define	CNT		    u.m3.m3i2
#define	REQUEST		u.m3.m3i2
#define	PROC_NR		u.m3.m3i3
#define	DEVICE		u.m3.m3i4
#define	POSITION	u.m3.m3l1
#define	BUF	    	u.m3.m3p2
#define	OFFSET		u.m3.m3i2 
#define	WHENCE		u.m3.m3i3 

#define	PID		    u.m3.m3i2
#define	STATUS		u.m3.m3i1
#define	RETVAL		u.m3.m3i1

// fs
#define FD_STDIN  0
#define FD_STDOUT 1
#define FD_STDERR 2

// ================== utility macros ================
#define	max(a,b)	((a) > (b) ? (a) : (b))
#define	min(a,b)	((a) < (b) ? (a) : (b))

#endif