#ifndef _UTILS_H
#define _UTILS_H

#include <const.h>
#include <sys/types.h>
#include <string.h>

#define	max(a,b)	((a) > (b) ? (a) : (b))
#define	min(a,b)	((a) < (b) ? (a) : (b))

#define	phys_copy	memcpy
#define	phys_set	memset

// used by non-kernel process before tty is running.
void printx(const char *fmt, ...); 
void assertion_failurex(char *exp, char *file, char *base_file, int line);
#define assertx(exp)  if (exp) ; \
        else assertion_failurex(#exp, __FILE__, __BASE_FILE__, __LINE__)
void spinx(char* func_name);
void panicx(const char *fmt, ...);

// used by user space process after tty is running.
void assertion_failure(char *exp, char *file, char *base_file, int line);
#define assert(exp)  if (exp) ; \
        else assertion_failure(#exp, __FILE__, __BASE_FILE__, __LINE__)
void spin(char* func_name);
void panic(const char *fmt, ...);

int vsprintf(char *buf, const char *fmt, va_list args);
// drivers/servers can use below syscall
uint8_t inb(uint16_t port);
void outb(uint16_t port, uint8_t value);
void portread (uint16_t port, void* buf, int n);
void portwrite(uint16_t port, void* buf, int n);
void enableirq(int irq);
void setintr();
void clearintr();
void setcursor(unsigned int position);
unsigned int getcursor();
void setvideostartaddr(unsigned int addr);
uint8_t retrive_scan_code_from_kb_buf();
void setkeypressed(int value);

// should only used by kernel and drivers, 
// TODO: move to kernel, add driver version of it to use kcall 
void inform_int(int task_nr);
 
// system calls by direct kcall

int kc_ticks();
void udelay(int milli_sec);
struct kmessage;
void reset_msg(struct kmessage* p);
int send_recv(int function, int src_dest, struct kmessage* p_msg);
 
// system calls by messaging


#endif