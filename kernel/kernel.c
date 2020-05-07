#include "screen.h"
#include "system.h"
#include "klib.h"
#include "pmmgr.h"

// TODO: need to replace mem_size with get_mem_size() function during boot loader.
// hardcode mem size to 256M
#define MEM_SIZE 0x10000000 
#define KERNEL_SIZE 0x100000 // TODO: move to get_kernel_size()
#define KERNEL_BASE 0x100000

void kmain(){
   
   clear_screen();
   kprint("minos\nkernel v0.1\n");

   idt_install();  
   isrs_install(); // setup handlers for NMI and exceptions (0-31)
   irq_install();  // setup handlers for interrupts (32 - 47 for irq 0 - 15)
   
   // physical mem layout
   // |kernel|physical mem bitmap| page directory | page tables | ...
   // | 1M   |  x bytes          | 
   // initialize the physical memory manager
   // we place the memory bit map used by the PMM at the end of the kernel in memory
   pmmgr_init(MEM_SIZE/1024, KERNEL_BASE + KERNEL_SIZE);
   // test physical memory alloc
   uint32_t* p = (uint32_t*)pmmgr_alloc_block();
   kprint("\nalloc 4KB at 0x");
   kprint(itoa(p));
   pmmgr_free_block(p); // free_blocks(p, num);
   kprint("\nfree 4KB memory\n");

   page_install();
   kprint("paging function enabled.\n");

   timer_install();
   keyboard_install();  
   
   __asm__ __volatile__ ("sti"); // enable interupt from here

   //kprint("wait started.\n");
   //timer_wait(18*2); // 2 seconds wait
   //kprint("wait ended.\n");
   
   while(1);
}