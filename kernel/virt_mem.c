#include "virt_mem.h"
#include "types.h"
#include "ke_asm_utils.h"
#include "klib.h"

// ============= PTE =================
inline void pt_entry_add_attrib (pt_entry* e, uint32_t attrib) {
	*e |= attrib;
}

inline void pt_entry_del_attrib (pt_entry* e, uint32_t attrib) {
	*e &= ~attrib;
}

inline void pt_entry_set_frame (pt_entry* e, physical_addr addr) {
	*e = (*e & ~I86_PTE_FRAME) | addr;
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
	*e = (*e & ~I86_PDE_FRAME) | addr;
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

inline void pd_entry_enable_global (pd_entry e) {

}

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
	if (!dir) return false;
	_cur_directory = dir;
	vmmgr_load_pdbr (_cur_pdbr);
	return true;
}

struct pdirectory* vmmgr_get_directory () {
	return _cur_directory;
}

bool_t vmmgr_alloc_page (pt_entry* e) {
	//! allocate a free physical frame
	void* p = pmmgr_alloc_block ();
	if (!p)
		return false;

	//! map it to the page
	pt_entry_set_frame (e, (physical_addr)p);
	pt_entry_add_attrib (e, I86_PTE_PRESENT);
	//doesent set WRITE flag...

	return true;
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
   if ( (*e & I86_PTE_PRESENT) != I86_PTE_PRESENT) {

      // page table not present, allocate it
      struct ptable* table = (struct ptable*) pmmgr_alloc_block ();
      if (!table)
         return;

      // clear page table
      memset ((char*)table, 0, sizeof(struct ptable));

      //! create a new entry
      pd_entry* entry = &pageDirectory->m_entries[PAGE_DIRECTORY_INDEX((uint32_t)virt)];

      //! map in the table (Can also just do *entry |= 3) to enable these bits
      pd_entry_add_attrib (entry, I86_PDE_PRESENT);
      pd_entry_add_attrib (entry, I86_PDE_WRITABLE);
      pd_entry_set_frame (entry, (physical_addr)table);
   }

   //! get table
   struct ptable* table = (struct ptable*) PAGE_GET_PHYSICAL_ADDRESS ( e );
   //! get page
   pt_entry* page = &table->m_entries [ PAGE_TABLE_INDEX ( (uint32_t) virt) ];

   //! map it in (Can also do (*page |= 3 to enable..)
   pt_entry_set_frame ( page, (physical_addr) phys);
   pt_entry_add_attrib ( page, I86_PTE_PRESENT);
}

void vmmgr_init () {
    // allocate default directory table
    struct pdirectory* dir = (struct pdirectory*)pmmgr_alloc_block(); //pmmgr_alloc_blocks(3); // TODO: ?? WHY 3?
    if (!dir) return;
    memset ((char*)dir, 0, sizeof (struct pdirectory)); 

    // allocate first page table, 4m from 0
    struct ptable* table0 = (struct ptable*) pmmgr_alloc_block ();
    if (!table0) return;
    memset ((char*)table0, 0, sizeof (struct ptable));

    // allocate 3g page table, 4m from 3g
    struct ptable* table3g = (struct ptable*) pmmgr_alloc_block ();
    if (!table3g) return;
    memset ((char*)table3g, 0, sizeof (struct ptable));

    //! 1st 4mb are idenitity mapped
    for (int i=0, frame=0x0, virt=0x00000000; i<1024; i++, frame+=4096, virt+=4096) {
        pt_entry page=0;
        pt_entry_add_attrib (&page, I86_PTE_PRESENT);
        pt_entry_set_frame  (&page, frame);

        //! ...and add it to the page table
        table0->m_entries [PAGE_TABLE_INDEX (virt) ] = page;
    }

    // TODO: depends on where the kernel is loaded, subject to change 
    //! map 1mb to 3gb 
    for (int i=0, frame=0x100000, virt=0xc0000000; i<1024; i++, frame+=4096, virt+=4096) {
        pt_entry page=0;
        pt_entry_add_attrib (&page, I86_PTE_PRESENT);
        pt_entry_set_frame  (&page, frame);

        //! ...and add it to the page table
        table3g->m_entries [PAGE_TABLE_INDEX (virt) ] = page;
    }
    
    // get 2 entries in dir table and set them up to point to our 2 tables
    pd_entry* entry0 = &dir->m_entries [PAGE_DIRECTORY_INDEX (0x00000000) ];
    pd_entry_add_attrib (entry0, I86_PDE_PRESENT);
    pd_entry_add_attrib (entry0, I86_PDE_WRITABLE);
    pd_entry_set_frame  (entry0, (physical_addr)table0);

    pd_entry* entry3g = &dir->m_entries [PAGE_DIRECTORY_INDEX (0xc0000000) ];
    pd_entry_add_attrib (entry3g, I86_PDE_PRESENT);
    pd_entry_add_attrib (entry3g, I86_PDE_WRITABLE);
    pd_entry_set_frame  (entry3g, (physical_addr)table3g);
    
    // store current PDBR
    _cur_pdbr = (physical_addr) &dir->m_entries;
        
    // switch to our page directory
    vmmgr_switch_pdirectory (dir);    
    // remember that as soon as paging is enabled, all address become virtual
    vmmgr_enable_paging ();
    // all addresses are virtual after this line.
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
	return (res & 0x80000000) ? false : true;
}

void vmmgr_load_pdbr (physical_addr addr) {
    load_pdbr(addr);
}

physical_addr vmmgr_get_pdbr() {
    return get_pdbr();
}
