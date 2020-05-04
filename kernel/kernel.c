#include "screen.h"
#include "system.h"

void kmain(){
   
   clear_screen();
   kprint("minos\nkernel v0.1\n");

   idt_install();  
   isrs_install(); // setup handlers for NMI and exceptions (0-31)
   irq_install();  // setup handlers for interrupts (32 - 47 for irq 0 - 15)
   
   page_install();

   timer_install();
   keyboard_install();  
   
   __asm__ __volatile__ ("sti"); // enable interupt from here

   //kprint("wait started.\n");
   //timer_wait(18*2); // 2 seconds wait
   //kprint("wait ended.\n");
   
   while(1);
}