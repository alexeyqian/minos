#ifndef KERNEL_H
#define KERNEL_H

// This is the master header for the kernel.

#define _POSIX_SOURCE 1 // tell headers to include POSIX files
#define _MINOS        1 // tell headers to include minos files
#define _SYSTEM       1 // tell headers that this is the kernel

#include "minos/config.h"// global config, must be first
//#include <ansi.h>		 // C style: ANSI or K&R, MUST be second
#include "sys/types.h"   // general system types
#include "minos/const.h" // MINOS specific constants
#include "minos/type.h"  // MINOS specific types
#include "minos/ipc.h"   // MINOS inter-process communication
#include "timers.h"      // watchdog timer management
#endif