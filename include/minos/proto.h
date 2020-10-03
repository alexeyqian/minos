#ifndef MINOS_PROTO_H
#define MINOS_PROTO_H

struct kmessage;

void syscall();   
int kcall(int function, void* msg); 
int sendrec(int function, int src_dest, struct kmessage* m); 

void drv_hd();    

void* va2la(int pid, void* va);

#endif