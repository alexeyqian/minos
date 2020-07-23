#ifndef MINOS_SYSCALL_H
#define MINOS_SYSCALL_H

int sys_get_ticks();
int sys_printx (int _unused1, int _unused2, char* s,             struct proc* p_proc);
int sys_write  (int _unused,  char* buf,    int len,             struct proc* p_proc);
int sys_sendrec(int function, int src_dest, struct s_message* m, struct proc* p_proc);

#endif