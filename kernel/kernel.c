#include "../drivers/screen.h"
#include "system.h"

// only gdt is setup in real mode
// idt and others are all setup in protected mode
void kmain(){
   
   clear_screen();
   print("minos\n");
   print("kernel v0.1\n");

   idt_install();  
   isrs_install(); // setup handlers for NMI and exceptions
   irq_install(); // setup handlers for interrupts
   page_install();

   __asm__ __volatile__ ("sti");

   timer_install();
   keyboard_install();   

   print("wait started.\n");
   timer_wait(18*2); // 2 seconds wait
   print("wait ended.\n");

   while(1);
}