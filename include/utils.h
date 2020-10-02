#ifndef _UTILS_H
#define _UTILS_H

#include <sys/types.h>
#include <minos/types.h>
#include <string.h>

#define	max(a,b)	((a) > (b) ? (a) : (b))
#define	min(a,b)	((a) < (b) ? (a) : (b))

#define	phys_copy	memcpy
#define	phys_set	memset

int vsprintf(char *buf, const char *fmt, va_list args);
// should only used by kernel and drivers, 
// TODO: move to kernel, add driver version of it to use kcall 
void inform_int(int task_nr);

// system calls by direct kcall
void printx(const char *fmt, ...); // TODO: rename to kc_print
int kc_ticks();
void udelay(int milli_sec);
int send_recv(int function, int src_dest, KMESSAGE* p_msg);
 
// system calls by messaging


#endif