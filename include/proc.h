#ifndef MINOS_PROC_H
#define MINSO_PROC_H

#include "types.h"
struct descriptor;

void init_descriptor(struct descriptor* p_desc, uint32_t base, uint32_t limit, uint16_t attribute);
void init_proc_table();
int getpid();
int wait(int* status);
void exit(int status);
int fork();

#endif