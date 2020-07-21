#ifndef _MINOS_IPC_H_
#define _MINOS_IPC_H_

struct MESSAGE;

void task_sys();
int sys_sendrec(int function, int src_dest, MESSAGE* m, struct proc* p);

#endif