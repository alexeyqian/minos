#ifndef PAGING_H
#define PAGING_H
#include "types.h"

// A linear address 'la' has a three-part structure as follows:
//
// +--------10------+-------10-------+---------12----------+
// | Page Directory |   Page Table   | Offset within Page  |
// |      Index     |      Index     |                     |
// +----------------+----------------+---------------------+
//  \--- PDX(la) --/ \--- PTX(la) --/ \---- PGOFF(la) ----/
//  \---------- PGNUM(la) ----------/
//
// The PDX, PTX, PGOFF, and PGNUM macros decompose linear addresses as shown.
// To construct a linear address la from PDX(la), PTX(la), and PGOFF(la),
// use PGADDR(PDX(la), PTX(la), PGOFF(la)).

// Page Directory Entry Format
// |31               12|11                        0|
// |page table address |AVAIL|G|S|0|A|D|W|U/S|R/W|P|
// P - PRESENT
// R/W - READ/WRITE
// U/S - USER/SUPERVISOR
// W - WRITE THROUGH
// D - CACHE DISABLED
// A - ACCESSED
// S - PAGE SIZE
// G - IGNORED

// Page Table Entry Format
// |31               12|11                      0|
// |page frame address |AVAIL|00|D|A|00|U/S|R/W|P|
// P - PRESENT
// R/W - READ/WRITE
// U/S - USER/SUPERVISOR
// D -DIRTY
// AVAIL - AVAILABLE FOR SYSTEMS PROGRAMER USE
// 0 INTEL RESERVED.

// page directory entry and page table entry have same format.
// int page table, the page frame address specifies the physical starting address of a page.
// int page directory, the page frame address is the address of a page table.
// pdbr - page directory base register.

#define PAGES_PER_TABLE 1024
#define PAGES_PER_DIR   1024
#define PAGE_SIZE       4096

#define PAGE_DIR_INDEX(x) (((x) >> 22) & 0x3ff)
#define PAGE_TABLE_INDEX(x) (((x) >> 12) & 0x3ff)
#define PAGE_INSIDE_OFFSET(x) (*x & ~0xfff)

// page directory / page table entry flags
#define PTE_P     0x001 // Present
#define PTE_W     0x002 //Writeable
#define PTE_U     0x004 // User
#define PTE_PWT   0x008 // Write-Through
#define PTE_PCD   0x010 // Cache-Disable
#define PTE_A     0x020 // Accessed
#define PTE_D     0x040 // Dirty
#define PTE_PS    0x080 // Page Size
#define PTE_G     0x100 // Global

typedef uint32_t page_pde;
typedef uint32_t page_pte;
typedef uint32_t virtual_addr;

struct page_dir{
    page_pde entries[PAGES_PER_DIR];
};
struct page_table{
    page_pte entries[PAGES_PER_TABLE];
};

#endif