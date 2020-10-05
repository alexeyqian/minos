#ifndef _UTILS_H
#define _UTILS_H

#include <const.h>
#include <sys/types.h>
#include <string.h>

#define	max(a,b)	((a) > (b) ? (a) : (b))
#define	min(a,b)	((a) < (b) ? (a) : (b))

#define	phys_copy	memcpy
#define	phys_set	memset

int sprintf(char* buf, const char* fmt, ...); // print to buf

// used by non-kernel process before tty is running.
void printx(const char *fmt, ...); 
void assertion_failurex(char *exp, char *file, char *base_file, int line);
#define assertx(exp)  if (exp) ; \
        else assertion_failurex(#exp, __FILE__, __BASE_FILE__, __LINE__)
void spinx(char* func_name);
void panicx(const char *fmt, ...);

int vsprintf(char *buf, const char *fmt, va_list args);
// drivers/servers can use below syscall
uint8_t inb(uint16_t port);
void outb(uint16_t port, uint8_t value);
void portread (uint16_t port, void* buf, int n);
void portwrite(uint16_t port, void* buf, int n);
void enableirq(int irq);

// should only used by kernel and drivers, 
// TODO: move to kernel, add driver version of it to use kcall 
void inform_int(int task_nr);

// system calls by direct kcall

int kc_ticks();
void udelay(int milli_sec);
struct kmessage;
int send_recv(int function, int src_dest, struct kmessage* p_msg);
 
// system calls by messaging


#endif