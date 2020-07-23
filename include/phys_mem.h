#ifndef MINOS_PHYS_MEM_H
#define MINOS_PHYS_MEM_H

#include "types.h"
#include "ktypes.h" 

#define MEM_REGION_COUNT 1 // TODO: update to 15

typedef uint32_t physical_addr;

void pmmgr_init();
void* pmmgr_alloc_block ();
void pmmgr_free_block (void*);
void* pmmgr_alloc_blocks (size_t);
void pmmgr_free_blocks (void*, size_t);
size_t pmmgr_memory_size ();
uint32_t pmmgr_used_block_count ();
uint32_t pmmgr_free_block_count ();
uint32_t pmmgr_max_block_count ();
uint32_t pmmgr_block_size ();

#endif