#ifndef PORTO_H
#define PROTO_H

#include <sys/types.h>
#include "ktypes.h"

// interrupt.c
void init_idt();
void irq_handler(int irq);
 void set_key_pressed(int value);
uint8_t read_from_kb_buf();

// kprintf.c
 void set_cursor(int offset);
 int get_cursor();
void kcls();
void kputs(char* str);
void kprintf(const char *fmt, ...);
void kassertion_failure(char *exp, char *file, char *base_file, int line);
#define kassert(exp)  if (exp) ; \
        else kassertion_failure(#exp, __FILE__, __BASE_FILE__, __LINE__)

void kpanic(const char *fmt, ...);
void kspin(char* func_name);
void never_here();

//boot_params.c
struct boot_params;
void read_boot_params(struct boot_params* bp);

// protect.c
void init_descriptor(struct descriptor* p_desc, uint32_t base, uint32_t limit, uint16_t attribute);

// proc.c
void init_proctable();

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

#endif