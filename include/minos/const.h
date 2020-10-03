#ifndef MINOS_CONST_H
#define MINOS_CONST_H

// ========= IRQ ==============
#define NR_IRQ          16

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

// TASK
#define NR_SYSCALLS     2

#define NR_TASKS        3
#define NR_PROCS        1
//#define NR_NATIVE_PROCS 4
#define TASK_CLOCK	    0  
#define TASK_SYS	    1 
#define TASK_HD		    2 

#define SVC_FS         10
//#define INIT            6 // first user proc

#define ANY		       (NR_TASKS + NR_PROCS + 10)
#define NO_TASK		   (NR_TASKS + NR_PROCS + 20)

// kernel space tasks
#define STACK_SIZE_CLOCK 0x8000  
#define STACK_SIZE_SYS   0x8000
#define STACK_SIZE_HD    0x8000

// user space services
#define STACK_SIZE_FS    0x8000
#define STACK_SIZE_TOTAL STACK_SIZE_CLOCK+STACK_SIZE_SYS+STACK_SIZE_HD \
	+STACK_SIZE_FS

// CLOCK
#define HZ              100

// TODO: add prefix: MSG_FUNC_
// IPC
#define SEND    1
#define RECEIVE 2
#define BOTH    3
#define INTERRUPT -10

// TODO: add prefix MSG_FLAG
// PROC: proc.p_flags
#define SENDING   0x02	// set when proc trying to send 
#define RECEIVING 0x04	// set when proc trying to recv 
#define WAITING   0x08  // proc waiting for the child to terminate
#define HANGING   0x10  // proc exits without being waited by parent
#define FREE_SLOT 0x20  // proc table entry is not used

// HD
#define INVALID_DRIVER	-20


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

#endif