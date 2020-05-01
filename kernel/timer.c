#include "../drivers/screen.h"
#include "system.h"

#define CLOCK_HZ 100
#define CLOCK_DIVIDOR 11931
int timer_ticks = 0;

void timer_handler(struct regs *r){
    timer_ticks++;
    // every 18 clocks - around 1 second,
    // we will display a message to the screen
    if(timer_ticks % CLOCK_HZ == 0)
        //print("One second has passed\n");
        print(timer_ticks);
}

void timer_wait(int ticks){
    unsigned long eticks;
    eticks = timer_ticks + ticks;
    while(timer_ticks < eticks) 
        ;    // busy waiting
}

// TODO: not used yet, for later to set timer to 100HZ
void timer_phase(int hz)
{
    int divisor = 1193180 / hz;       /* Calculate our divisor */
    port_byte_out(0x43, 0x36);             /* Set our command byte 0x36 */
    port_byte_out(0x40, divisor & 0xFF);   /* Set low byte of divisor */
    port_byte_out(0x40, divisor >> 8);     /* Set high byte of divisor */
}

void timer_install(){    
    timer_phase(CLOCK_DIVIDOR);
    timer_ticks = 0;
    irq_install_handler(0, timer_handler);
}