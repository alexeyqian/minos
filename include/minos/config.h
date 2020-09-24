#ifndef MINOS_CONFIG_H
#define MINOS_CONFIG_H

#define OS_RELEASE "1"
#define OS_VERSION "1.0"

#include "minos/sys_config.h"

#define MACHINE _MINIX_MACHINE
#define IBM_PC  _MACHINE_IBM_PC

#define NR_PROCS _NR_PROCS // = NR_USER_PROCS + NR_SYS_PROCS
#define NR_SYS_PROCS _NR_SYS_PROCS
#define NR_USR_PROCS (NR_PROCS - NR_SYS_PROCS)

#define NR_BUFS 128
#define NR_BUF_HASH 128

//#define OUTPUT_PROC_NR LOG_PROC_NR
#define NR_CONS     4 // system consoles (1-8)
#define NR_RS_LINES 0 // RS232 terminals (0-4)
#define NR_PTYS     0 // pseudo terminals (0-64)

#define INTEL _CHIP_INTEL
#define CHIP _MINIX_CHIP
// ...

#endif