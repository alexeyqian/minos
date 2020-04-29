#include "../drivers/screen.h"
#include "system.h"

void kmain(){
   
   clear_screen();
   print("minos\n");
   print("kernel v0.1\n");

   idt_install();
   isrs_install();
   irq_install();

   __asm__ __volatime__("sti");

   timer_install();
   keyboard_install();

   print("wait started.\n");
   timer_wait(18*2); // 2 seconds wait
   print("wait ended.\n");

   while(1);
}