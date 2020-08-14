#ifndef MINOS_VIRT_MEM_H
#define MINOS_VIRT_MEM_H

#include "types.h"
#include "ktypes.h"
#include "phys_mem.h"

// =========== PTE ===============
enum PAGE_PTE_FLAGS {

	I86_PTE_PRESENT			=	1,			//0000000000000000000000000000001
	I86_PTE_WRITABLE		=	2,			//0000000000000000000000000000010
	I86_PTE_USER			=	4,			//0000000000000000000000000000100
	I86_PTE_WRITETHOUGH		=	8,			//0000000000000000000000000001000
	I86_PTE_NOT_CACHEABLE	=	0x10,		//0000000000000000000000000010000
	I86_PTE_ACCESSED		=	0x20,		//0000000000000000000000000100000
	I86_PTE_DIRTY			=	0x40,		//0000000000000000000000001000000
	I86_PTE_PAT				=	0x80,		//0000000000000000000000010000000
	I86_PTE_CPU_GLOBAL		=	0x100,		//0000000000000000000000100000000
	I86_PTE_LV4_GLOBAL		=	0x200,		//0000000000000000000001000000000
   	I86_PTE_FRAME			=	0x7FFFF000 	//1111111111111111111000000000000
};

typedef uint32_t pt_entry;

void pt_entry_add_attrib(pt_entry* e, uint32_t aatrib);
void pt_entry_del_attrib(pt_entry* e, uint32_t attrib);
void pt_entry_set_frame(pt_entry*, physical_addr);
bool_t pt_entry_is_present(pt_entry e);
bool_t pt_entry_is_writable(pt_entry e);
physical_addr pt_entry_pfn(pt_entry e);

// ================ PDE =============
enum PAGE_PDE_FLAGS {

	I86_PDE_PRESENT			=	1,			//0000000000000000000000000000001
	I86_PDE_WRITABLE		=	2,			//0000000000000000000000000000010
	I86_PDE_USER			=	4,			//0000000000000000000000000000100
	I86_PDE_PWT				=	8,			//0000000000000000000000000001000
	I86_PDE_PCD				=	0x10,		//0000000000000000000000000010000
	I86_PDE_ACCESSED		=	0x20,		//0000000000000000000000000100000
	I86_PDE_DIRTY			=	0x40,		//0000000000000000000000001000000
	I86_PDE_4MB				=	0x80,		//0000000000000000000000010000000
	I86_PDE_CPU_GLOBAL		=	0x100,		//0000000000000000000000100000000
	I86_PDE_LV4_GLOBAL		=	0x200,		//0000000000000000000001000000000
   	I86_PDE_FRAME			=	0x7FFFF000 	//1111111111111111111000000000000
};

// page directery entry
typedef uint32_t pd_entry;

void pd_entry_add_attrib (pd_entry* e, uint32_t attrib);
void pd_entry_del_attrib (pd_entry* e, uint32_t attrib);
void pd_entry_set_frame (pd_entry*, physical_addr);
bool_t pd_entry_is_present (pd_entry e);
bool_t pd_entry_is_user (pd_entry);
bool_t pd_entry_is_4mb (pd_entry);
bool_t pd_entry_is_writable (pd_entry e);
physical_addr pd_entry_pfn (pd_entry e);
void pd_entry_enable_global (pd_entry e);

// =========== virtual memory manager ==========
typedef uint32_t virtual_addr;

#define PAGES_PER_TABLE 1024
#define PAGES_PER_DIR   1024
#define PAGE_DIRECTORY_INDEX(x) (((x) >> 22) & 0x3ff)
#define PAGE_TABLE_INDEX(x) (((x) >> 12) & 0x3ff)
#define PAGE_GET_PHYSICAL_ADDRESS(x) (*x & ~0xfff)

struct ptable{
    pt_entry m_entries[PAGES_PER_TABLE];
};
struct pdirectory{
    pd_entry m_entries[PAGES_PER_DIR];
};

void vmmgr_init ();
//! maps phys to virtual address
void vmmgr_map_page (void* phys, void* virt);
bool_t vmmgr_alloc_page (pt_entry*);
void vmmgr_free_page (pt_entry* e);
bool_t vmmgr_switch_pdirectory (struct pdirectory*);
//get current page directory
struct pdirectory* vmmgr_get_directory ();
// flushes a cached translation lookaside buffer (TLB) entry
void vmmgr_flush_tlb_entry (virtual_addr addr);
// clears a page table
void vmmgr_ptable_clear (struct ptable* p);
//! convert virtual address to page table index
uint32_t vmmgr_ptable_virt_to_index (virtual_addr addr);
//! get page entry from page table
pt_entry* vmmgr_ptable_lookup_entry (struct ptable* p,virtual_addr addr);
//! convert virtual address to page directory index
uint32_t vmmgr_pdirectory_virt_to_index (virtual_addr addr);
//! clears a page directory table
void vmmgr_pdirectory_clear (struct pdirectory* dir);
//! get directory entry from directory table
pd_entry* vmmgr_pdirectory_lookup_entry (struct pdirectory* p, virtual_addr addr);

void vmmgr_enable_paging();
void vmmgr_disable_paging();
bool_t vmmgr_is_paging ();
void vmmgr_load_pdbr(physical_addr);
physical_addr vmmgr_get_pdbr();

#endif