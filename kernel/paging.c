#include "paging.h"
#include "system.h"

extern unsigned long read_cr0();
extern void write_cr0();
extern unsigned long read_cr3();
extern void write_cr3();

uint32_t _cur_dir = 0;

void page_install(){
    unsigned long *page_directory = (unsigned long*)0x9c000;
    // page table comes right after page directory
    unsigned long *page_table = (unsigned long*)0x9d000;

    unsigned long address = 0;
    // file page table, map the first 4MB of memory
    for(int i = 0; i < 1024; i++){
        // attribute set to: supervisor level, read/write, present 011
        page_table[i] = address | 3;
        address = address + 4096; 
    }

    // fill the page directory entries
    // fill the first entry of the page directory
    page_directory[0] = (unsigned long)page_table;
    page_directory[0] |= 3; // attribute set to 011

    // set attribute of the remaining 1023 entries to not-present 010
    for(int j = 1; j < 1024; j++)
        page_directory[j] = 0 | 2;
    
    write_cr3(page_directory);
    write_cr0(read_cr0() | 0x80000000); // set the page bit in cr0 to 1
}

bool vmmgr_alloc_page(page_pte* e){    
    void *p = pmmgr_alloc_block();
    if(!p) return false;

    page_pte_set_frame(e, (physical_addr)p);
    page_pte_add_attr(e, PTE_P);

    return true;
}

void vmmgr_free_page(pt_entry* e){
    void* p = (void*)pt_entry_pfn(*e);
    if(p)
        pmmgr_free_block(p);
    pt_entry_del_attrib(e, I86_PTE_PRESENT);
}

inline pt_entry* vmmgr_ptable_lookup_entry(ptable* p, virtual_addr addr){
    if(!p) return 0;
    return &p->m_entries[PAGE_TABLE_INDEX(addr)];
}

inline pd_entry* vmmgr_ptable_lookup_entry(pdirectory* p, virtual_addr addr){
    if(!p) return 0;
    return &p->m_entries[PAGE_TABLE_INDEX(addr)];
}

inline bool vmmgr_switch_pdirectory(pdirectory* dir){
    if(!dir) return false;

    _cur_directory = dir;
    pnmgr_load_PDBR(_cur_directory);
    return true;
}

pdirectory* vmmgr_get_directory(){
    return _cur_directory;
}

void vmmgr_map_page(void* phys, void virt){
    pdirectory* page_directory = vmmgr_get_directory();

    pd_entry* e = &page_directory->m_entries[PAGE_DIRECTORY_INDEX((uint32_t)virt)];
    if((*e & I86_PTE_PRESENT) != I86_PTE_PRESENT){ // create new page
        //! page table not present, allocate it
      ptable* table = (ptable*) pmmngr_alloc_block ();
      if (!table)
         return;

      //! clear page table
      memset (table, 0, sizeof(ptable));

      //! create a new entry
      pd_entry* entry =
         &pageDirectory->m_entries [PAGE_DIRECTORY_INDEX ( (uint32_t) virt) ];

      //! map in the table (Can also just do *entry |= 3) to enable these bits
      pd_entry_add_attrib (entry, I86_PDE_PRESENT);
      pd_entry_add_attrib (entry, I86_PDE_WRITABLE);
      pd_entry_set_frame (entry, (physical_addr)table);
    }

    ptable* table = (ptable*) PAGE_GET_PHYSICAL_ADDRESS(e);
    pt_entry* page = &table->m_entries [ PAGE_TABLE_INDEX ( (uint32_t) virt) ];

   //! map it in (Can also do (*page |= 3 to enable..)
   pt_entry_set_frame ( page, (physical_addr) phys);
   pt_entry_add_attrib ( page, I86_PTE_PRESENT);
}

// map 1MB physical memory to 3G virtual address space for the kernel
void vmmgr_init(){
    // allocate default page table
    page_table* table = (page_table*) pmmgr_alloc_block();
    if(!table) return;

    // allocate 3GB page table
    page_table* table2 = (page_table*) pmmgr_alloc_block();
    if(!table2) return;

    vmmgr_ptable_clear(table);

    //This parts a little tricky. Remember that as soon as paging is enabled, all address become virtual? 
    // This poses a problem. To fix this, we must map the virtual addresses to the same physical addresses 
    // so they refer to the same thing. This is idenitity mapping.
    // The below code idenitity maps the page table to the first 4MB of physical memory
    // first 4MB are identity mapped
    int frame = 0x0;
    int virt = 0x00000000;
    for(int i = 0; i < 1024; i++){
        // create a new page
        pt_entry page = 0;
        pt_entry_add_attrib(&page, I86_PTE_PRESENT);
        pt_entry_set_frame(&page, frame);
        table2->m_entries[PAGE_TABLE_INDEX(virt)] = page;

        frame += PAGE_SIZE;
        virt += PAGE_SIZE;
    }

    // remap the kernel from 1M physical to 3GB virtual
    // this allows the kernel to continue running at 3GB virtual address
    frame = 0x100000;
    virt = 0xc0000000;
    for(int i = 0; i < 1024; i++){
        // create a new page
        pt_entry page = 0;
        pt_entry_add_attrib(&page, I86_PTE_PRESENT);
        pt_entry_set_frame(&page, frame);

        frame += PAGE_SIZE;
        virt += PAGE_SIZE;
    }

    // create default directory table
    pdirectory* dir = (pdirectory*) pmmgr_allock_blocks(3);
    if(!dir) return;
    memset(dir, 0, sizeof(pdirectory));

    pd_entry* entry = &dir->m_entries [PAGE_DIRECTORY_INDEX (0xc0000000) ];
	pd_entry_add_attrib (entry, I86_PDE_PRESENT);
	pd_entry_add_attrib (entry, I86_PDE_WRITABLE);
	pd_entry_set_frame (entry, (physical_addr)table);

	pd_entry* entry2 = &dir->m_entries [PAGE_DIRECTORY_INDEX (0x00000000) ];
	pd_entry_add_attrib (entry2, I86_PDE_PRESENT);
	pd_entry_add_attrib (entry2, I86_PDE_WRITABLE);
	pd_entry_set_frame (entry2, (physical_addr)table2);

    _cur_pdbr = (physical_addr) &dir->m_entries;
    wmmgr_switch_pdirectory(dir);
    pmmgr_paging_enable(true);

}
