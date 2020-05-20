#ifndef _MINOS_FUNC_DEF_H
#define _MINOS_FUNC_DEF_H

#include "types.h"
#include "ktypes.h"

void init_descriptor(struct descriptor* p_desc, uint32_t base, uint32_t limit, uint16_t attribute);

uint32_t seg_to_physical(uint16_t seg);
void move_gdt_from_loader_mem_to_kernel_mem();

void init_tss();
void init_tss_descriptor_in_gdt();
void init_ldt_descriptors_in_dgt();
void init_proc_table_from_task_table();

void delay(int time);
void restart();

#endif