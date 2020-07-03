#ifndef _MINOS_PHYS_MEM_H_
#define _MINOS_PHYS_MEM_H_

#include "ktypes.h" // should be stdint.h

typedef uint32_t physical_addr;

void pmmngr_init (size_t, physical_addr);

//! enables a physical memory region for use
void	pmmngr_init_region (physical_addr, size_t);

//! disables a physical memory region as in use (unuseable)
void	pmmngr_uninit_region (physical_addr base, size_t);

void*	pmmngr_alloc_block ();

void	pmmngr_free_block (void*);

void*	pmmngr_alloc_blocks (size_t);

void	pmmngr_free_blocks (void*, size_t);

size_t pmmng_memory_size ();

uint32_t pmmngr_used_block_count ();

uint32_t pmmngr_free_block_count ();

uint32_t pmmngr_max_block_count ();

uint32_t pmmngr_block_size ();

//extern	void	pmmngr_paging_enable (bool);

//extern	bool	pmmngr_is_paging ();

//extern	void	pmmngr_load_PDBR (physical_addr);

//extern	physical_addr pmmngr_get_PDBR ();
#endif