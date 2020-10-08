#include "kernel.h"

PUBLIC void init_clock(){ // init 8253 PIT
	out_byte(TIMER_MODE, RATE_GENERATOR);
	out_byte(TIMER0, (uint8_t) (TIMER_FREQ/HZ) );
	out_byte(TIMER0, (uint8_t) ((TIMER_FREQ/HZ) >> 8));
	enable_irq(CLOCK_IRQ);	
}

// priority is fixed value, ticks is counting down.
// when all processes ticks are 0, then reset ticks to it's priority.
PUBLIC void schedule(){
	struct proc* p;
	int greatest_ticks = 0;

	while(!greatest_ticks){
		for( p = proc_table; p < proc_table + PROCTABLE_SIZE; p++)
			if(p->p_flags == 0){
				if(p->ticks > greatest_ticks){
					greatest_ticks = p->ticks;
					p_proc_ready = p;
				}
			}

		if(!greatest_ticks)
			for(p = proc_table; p < proc_table + PROCTABLE_SIZE; p++)
				if (p->p_flags == 0)
					p->ticks = p->priority;
	}
}

/*
// round robin version of scheduler
PRIVATE void clock_handler_not_used(int irq){
	ticks++;
	if(k_reenter != 0){
		return;
	}

	// round robin process scheduler.
	p_proc_ready++;
	if(p_proc_ready >= proc_table + PROCTABLE_SIZE)
		p_proc_ready = proc_table;
}*/


PUBLIC void task_clock(){
    kprintf(">>> 1. task clock is running.\n");
    while(TRUE){}
}