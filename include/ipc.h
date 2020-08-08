#ifndef MINOS_IPC_H
#define MINOS_IPC_H

#define SEND    1
#define RECEIVE 2
#define BOTH    3

#define SENDING   0x02	// set when proc trying to send 
#define RECEIVING 0x04	// set when proc trying to recv 
#define WAITING   0x08  // proc waiting for the child to terminate
#define HANGING   0x10  // proc exits without being waited by parent
#define FREE_SLOT 0x20  // proc table entry is not used

struct s_message;
struct proc;

void task_sys();
int send_recv(int function, int src_dest, struct s_message* msg);
int get_ticks();
void inform_int(int task_nr);
void dump_msg(const char * title, struct s_message* m);
#endif