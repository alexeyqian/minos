#ifndef PORTO_H
#define PROTO_H

#include <sys/types.h>
#include "ktypes.h"

// interrupt.c
void init_idt();
void irq_handler(int irq);
void put_irq_handler(int irq, pf_irq_handler_t handler);

// kprintf.c
void kcls();
void kputs(char* str);

void kpanic(const char *fmt, ...);
void kspin(char* func_name);
void never_here();

// interrupt.c
void init_idt();
void irq_handler(int irq);
void put_irq_handler(int irq, pf_irq_handler_t handler);

//boot_params.c
struct boot_params;
void read_boot_params(struct boot_params* bp);

// protect.c
void init_descriptor(struct descriptor* p_desc, uint32_t base, uint32_t limit, uint16_t attribute);

// proc.c
void init_proc_table();

// ipc.c
struct kmessage;
struct proc;
// sys_kcall and sys_sendrec MUST have same parameter types (pointers are same types)
int sys_kcall(int _unused1, int _unused2, void* msg, struct proc* p_proc);
int sys_sendrec(int function, int src_dest, struct kmessage* m, struct proc* p_proc);
#define proc2pid(x) (x - proc_table)
#define	phys_copy	memcpy
#define	phys_set	memset

// clock.c
void task_clock(); 
void init_clock(); 
void schedule();

void task_sys();   // system.c

// TODO: move to services/fs
void svc_fs();

// pm.c
/*int getpid();
int wait(int* status);
void exit(int status);
int fork();
int exel(const char* path, const char *arg, ...);
int execv(const char* path, char* argv[]);
int execl(const char* path, const char *arg, ...);
*/
#endif