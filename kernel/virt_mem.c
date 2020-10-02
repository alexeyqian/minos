#include "virt_mem.h"
#include <sys/types.h>
#include <minos/proto.h>
#include <string.h>
#include <utils.h>
#include "ke_asm_utils.h"
#include "ktypes.h"
#include "proto.h"

// ============= PTE =================
inline void pt_entry_add_attrib (pt_entry* e, uint32_t attrib) {
	*e |= attrib;
}

inline void pt_entry_del_attrib (pt_entry* e, uint32_t attrib) {
	*e &= ~attrib;
}

inline void pt_entry_set_frame (pt_entry* e, physical_addr addr) {
	*e = (*e & ~(uint32_t)I86_PTE_FRAME) | addr;
}

inline bool_t pt_entry_is_present (pt_entry e) {
	return e & I86_PTE_PRESENT;
}

inline bool_t pt_entry_is_writable (pt_entry e) {
	return e & I86_PTE_WRITABLE;
}

inline physical_addr pt_entry_pfn (pt_entry e) {
	return e & I86_PTE_FRAME;
}

// =========== PDE ==============
inline void pd_entry_add_attrib (pd_entry* e, uint32_t attrib) {
	*e |= attrib;
}

inline void pd_entry_del_attrib (pd_entry* e, uint32_t attrib) {
	*e &= ~attrib;
}

inline void pd_entry_set_frame (pd_entry* e, physical_addr addr) {
	*e = (*e & ~(uint32_t)I86_PDE_FRAME) | addr;
}

inline bool_t pd_entry_is_present (pd_entry e) {
	return e & I86_PDE_PRESENT;
}

inline bool_t pd_entry_is_writable (pd_entry e) {
	return e & I86_PDE_WRITABLE;
}

inline physical_addr pd_entry_pfn (pd_entry e) {
	return e & I86_PDE_FRAME;
}

inline bool_t pd_entry_is_user (pd_entry e) {
	return e & I86_PDE_USER;
}

inline bool_t pd_entry_is_4mb (pd_entry e) {
	return e & I86_PDE_4MB;
}
/*
inline void pd_entry_enable_global (pd_entry e) {

}*/

// ============== VMMGR =================

//! page table represents 4mb address space
#define PTABLE_ADDR_SPACE_SIZE 0x400000
//! directory table represents 4gb address space
#define DTABLE_ADDR_SPACE_SIZE 0x100000000
//! page sizes are 4k
#define PAGE_SIZE 4096

//! current directory table
struct pdirectory* _cur_directory=0;
//! current page directory base register
physical_addr	_cur_pdbr=0;

inline pt_entry* vmmgr_ptable_lookup_entry (struct ptable* p,virtual_addr addr) {

	if (p)
		return &p->m_entries[ PAGE_TABLE_INDEX (addr) ];
	return 0;
}

inline pd_entry* vmmgr_pdirectory_lookup_entry (struct pdirectory* p, virtual_addr addr) {

	if (p)
		return &p->m_entries[ PAGE_TABLE_INDEX (addr) ];
	return 0;
}

inline bool_t vmmgr_switch_pdirectory (struct pdirectory* dir) {
	if (!dir) return FALSE;
	_cur_directory = dir;
	vmmgr_load_pdbr (_cur_pdbr);
	return TRUE;
}

struct pdirectory* vmmgr_get_directory () {
	return _cur_directory;
}

bool_t vmmgr_alloc_page (pt_entry* e) {
	//! allocate a free physical frame
	void* p = pmmgr_alloc_block ();
	if (!p)
		return FALSE;

	//! map it to the page
	pt_entry_set_frame (e, (physical_addr)p);
	pt_entry_add_attrib (e, I86_PTE_PRESENT);
	//doesent set WRITE flag...

	return TRUE;
}

void vmmgr_free_page (pt_entry* e) {

	void* p = (void*)pt_entry_pfn (*e);
	if (p)
		pmmgr_free_block (p);

	pt_entry_del_attrib (e, I86_PTE_PRESENT);
}

void vmmgr_map_page (void* phys, void* virt) {

   // get page directory
   struct pdirectory* pageDirectory = vmmgr_get_directory ();

   // get page table
   pd_entry* e = &pageDirectory->m_entries[PAGE_DIRECTORY_INDEX ((uint32_t) virt) ];
   if ( (*e & I86_PTE_PRESENT) != I86_PTE_PRESENT) { // page table not present, allocate it
      struct ptable* table = (struct ptable*) pmmgr_alloc_block ();
      if (!table) return;

      memset ((char*)table, 0, sizeof(struct ptable));

      // set a new entry
      pd_entry* entry = &pageDirectory->m_entries[PAGE_DIRECTORY_INDEX((uint32_t)virt)];
      pd_entry_add_attrib (entry, I86_PDE_PRESENT);
      pd_entry_add_attrib (entry, I86_PDE_WRITABLE);
      pd_entry_set_frame (entry, (physical_addr)table);
   }

   struct ptable* table = (struct ptable*) PAGE_GET_PHYSICAL_ADDRESS ( e );
   pt_entry* page = &table->m_entries [ PAGE_TABLE_INDEX ( (uint32_t) virt) ];
   pt_entry_set_frame ( page, (physical_addr) phys);
   pt_entry_add_attrib ( page, I86_PTE_PRESENT);
}

PRIVATE void map_page_table(struct pdirectory* dir, uint32_t paddr_base, uint32_t vaddr_base){
    struct ptable* table0 = (struct ptable*) pmmgr_alloc_block();
    kprintf(">>> map_page_table:: table0 allocated at: 0x%x", table0);
    if (!table0) return;
    memset ((char*)table0, 0, sizeof (struct ptable));
    
    for (uint32_t i = 0, frame = paddr_base, virt = vaddr_base; i < 1024; i++, frame += 4096, virt += 4096) {
        pt_entry page = 0;
        pt_entry_add_attrib (&page, I86_PTE_PRESENT);
        pt_entry_add_attrib (&page, I86_PTE_USER);
        pt_entry_add_attrib (&page, I86_PTE_WRITABLE);
        //kassert(frame >= 0);
        pt_entry_set_frame  (&page, frame);
        table0->m_entries [PAGE_TABLE_INDEX (virt) ] = page;
    }

    pd_entry* entry0 = &dir->m_entries [PAGE_DIRECTORY_INDEX (vaddr_base) ];
    pd_entry_add_attrib (entry0, I86_PDE_PRESENT);
    pd_entry_add_attrib (entry0, I86_PDE_WRITABLE);
    pd_entry_add_attrib (entry0, I86_PDE_USER);
    pd_entry_set_frame  (entry0, (physical_addr)table0);
}

void vmmgr_init () {
    struct pdirectory* dir = (struct pdirectory*)pmmgr_alloc_block(); 
    kprintf(">>> vmmgr_init:: dir allocated at: 0x%x", dir);
    if (!dir) return;
    memset ((char*)dir, 0, sizeof (struct pdirectory)); 

    // map physical mem 0 - 4M, idenitity mapped
    map_page_table(dir, 0x0, 0x0);  
    kprintf(">>> identical map: 0 - 4M\n");
    // map 4-8m, identity mapped  
    //map_page_table(dir, 0x400000, 0x400000);

    // map physical 1M - 1M + kernel_size 3G - 3G + kernel_size
    // map_page_table(dir, 0x100000, 0xc0000000);  
    
    _cur_pdbr = (physical_addr) &dir->m_entries;        
    // switch to our page directory
    vmmgr_switch_pdirectory (dir);    
    // remember that as soon as paging is enabled, all address become virtual
    vmmgr_enable_paging ();
    kprintf(">>> =========== all addresses are virtual after this line. =============\n");
}

// =========== functions using asm ================
void vmmgr_flush_tlb_entry (virtual_addr addr) {
    flush_tlb_entry(addr);
}

void vmmgr_enable_paging(){
    enable_paging();
}

void vmmgr_disable_paging(){
    disable_paging();
}

bool_t vmmgr_is_paging () {
	uint32_t res= get_cr0();
	return (res & 0x80000000) ? FALSE : TRUE;
}

void vmmgr_load_pdbr (physical_addr addr) {
    load_pdbr(addr);
}

physical_addr vmmgr_get_pdbr() {
    return get_pdbr();
}
