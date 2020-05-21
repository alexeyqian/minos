#ifndef _MINOS_FUNC_DEF_H
#define _MINOS_FUNC_DEF_H

#include "types.h"
#include "ktypes.h"

uint32_t seg_to_physical(uint16_t seg);
void init_tss();
void init_tss_descriptor_in_gdt();
void init_ldt_descriptors_in_dgt();
void init_proc_table_from_task_table();

void delay(int time);
void restart();

#endif