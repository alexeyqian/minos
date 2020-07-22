#ifndef MINOS_IPC_H
#define MINOS_IPC_H

#define SEND    1
#define RECEIVE 2
#define BOTH    3
#define RETVAL u.m3.m3i1
#define SENDING   0x02	/* set when proc trying to send */
#define RECEIVING 0x04	/* set when proc trying to recv */

struct s_message;
struct proc;

void task_sys();
int send_recv(int function, int src_dest, struct s_message* msg);
int get_ticks2();
#endif