#ifndef MINOS_PROC_H
#define MINSO_PROC_H

void init_proc_table();
int getpid();
int wait(int* status);
void exit(int status);
int fork();

#endif