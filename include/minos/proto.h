#ifndef MINOS_PROTO_H
#define MINOS_PROTO_H

struct kmessage;

void syscall();   
int kcall(int function, void* msg); 
int sendrec(int function, int src_dest, struct kmessage* m); 

// TODO: should be moved to kernel/proto.h
// after drivers are moved to userspace.
void kprintf(const char *fmt, ...);
void kassertion_failure(char *exp, char *file, char *base_file, int line);
#define kassert(exp)  if (exp) ; \
        else kassertion_failure(#exp, __FILE__, __BASE_FILE__, __LINE__)

void drv_hd();    

void* va2la(int pid, void* va);

#endif