#include "../drivers/screen.h"
#include "system.h"

int timer_ticks = 0;

void timer_handler(struct regs *r){
    timer_ticks++;
    // every 18 clocks - around 1 second,
    // we will display a message to the screen
    if(timer_ticks % 18 == 0)
        print("One second has passed\n");
}

void timer_wait(int ticks){
    unsigned long eticks;
    eticks = timer_ticks + ticks;
    while(timer_ticks < eticks) 
        ;    // busy waiting
}

void timer_install(){
    timer_ticks = 0;
    irq_install_handler(0, timer_handler);
}