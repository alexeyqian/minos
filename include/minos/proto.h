#ifndef MINOS_PROTO_H
#define MINOS_PROTO_H

struct kmessage;

void syscall();   
int kcall(int function, void* msg); 
int sendrec(int function, int src_dest, struct kmessage* m); 
void init();

void drv_hd();    

void* va2la(int pid, void* va);

int getpid();
int wait(int* status);
void exit(int status);
int fork();
int exel(const char* path, const char *arg, ...);
int execv(const char* path, char* argv[]);
int execl(const char* path, const char *arg, ...);

#endif