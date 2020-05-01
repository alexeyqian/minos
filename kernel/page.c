#include "system.h"
// page directory has to be 4K aligned (last 12 bits have to all 0s), 
// we choose 0x9C000 here, and makre sure it's matching kernel address

void page_install(){
    unsigned long *page_directory = (unsigned long *)0x9c000;
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
    page_directory[0] = page_table;
    page_directory[0] |= 3; // attribute set to 011

    // set attribute of the remaining 1023 entries to not-present 010
    for(int j = 1; j < 1024; j++)
        page_directory[j] = 0 | 2;

        extern read_cr0;
    extern write_cr0;
    extern read_cr3;
    extern write_cr3;
    write_cr3(page_directory);
    write_cr0(read_cr0() | 0x80000000); // set the page bit in cr0 to 1
}
