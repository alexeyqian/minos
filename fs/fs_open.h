#ifndef MINOS_FS_OPEN_H
#define MINOS_FS_OPEN_H

struct s_message;
struct proc;

int   do_open(struct s_message* pmsg, struct proc* caller);
int  do_close(struct s_message* pmsg, struct proc* caller);
int  do_lseek(struct s_message* pmsg, struct proc* caller);
int   do_rdwt(struct s_message* pmsg, struct proc* caller);
int do_unlink(struct s_message* pmsg);
void reset_filedesc_table();

#endif