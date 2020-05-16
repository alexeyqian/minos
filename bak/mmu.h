#ifndef MMU_H
#define MMU_H

// This file contains definitions for the x86 memory management unit: MMU
// including paging and segmentation related data structures and constants.

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

#define PDXSHIFT 22
#define PTXSHIFT 12

#define PGNUM(la) (((uintptr_t)(la)) >> PTXSHIFT)
#define PDX(la) ((((uintptr_t)(la)) >> PDXSHIFT) & 0x3FF)
#define PTX(la) ((((uintptr_t)(la)) >> PTXSHIFT) & 0X3FF)
#define PGOFF(la) (((uintptr_t)(la)) & 0xFFF)
#define PGADDR(d, t, o) ((void *)((d) << PDXSHIFT | (t) << PTXSHIFT | (o)))

#define N_PD_ENTRIES 1024
#define N_PT_ENTRIES 1024

#define PGSIZE 4096
#define PGSHIFT 12

#define PTSIZE (PGSIZE*N_PT_ENTRIES)
#define PTSHIFT 22

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

// The PTE_AVAIL bits aren't used by the kernel or interpreted by 
// the hardware, so user processes are allowed to set them arbitrarily
#define PTE_AVAIL 0xE00 // available for software use

#define PTE_SYSCALL (PTE_AVAIL | PTE_P | PTE_W | PTE_U)

#define PTE_ADDR(pte)  ((physaddr_t)(pte) & ~0xFFF)
#define PTE_FALGS(pte) ((physaddr_t)(pte) &  0xFFF)

#define CR0_PE  0x00000001 // Protection Enable
// ...

#define FL_CF        0x00000001 // Carry Flag
#define FL_PF        0x00000004 // Parity Flag
// ...


#endif