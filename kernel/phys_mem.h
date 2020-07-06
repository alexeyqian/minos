#ifndef _MINOS_PHYS_MEM_H_
#define _MINOS_PHYS_MEM_H_

#include "ktypes.h" // should be stdint.h

#define MEM_REGION_COUNT 1 // TODO: update to 15

typedef uint32_t physical_addr;

struct mem_region{
	uint32_t start_low;
	uint32_t start_high;
	uint32_t size_low;
	uint32_t size_high;
	uint32_t type;
	//uint32_t acpi_3_0; // TODO
};

void pmmgr_init(size_t, physical_addr);
//! enables a physical memory region for use
void pmmgr_init_region(physical_addr, size_t);
//! disables a physical memory region as in use (unuseable)
void pmmgr_uninit_region (physical_addr base, size_t);
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