#ifndef I_MINOS_CONST_H_
#define I_MINOS_CONST_H_

#define EXTERN  extern
#define PRIVATE static
#define PUBLIC 
#define FORWARD static

#define TRUE  1
#define FALSE 0

#define NULL ((void *)0)

#define SEGMENT_TYPE 0xFF00
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


#define CLICK_SIZE   1024 // unit in which memory is allocated
#define CLICK_SHIFT    10 // log2 of CLICK_SIZE
#endif