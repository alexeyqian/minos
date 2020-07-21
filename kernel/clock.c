#include "clock.h"
#include "types.h"
#include "ktypes.h"
#include "klib.h"
#include "global.h"


void enable_clock(){ // init 8253 PIT
	out_byte(TIMER_MODE, RATE_GENERATOR);
	out_byte(TIMER0, (uint8_t) (TIMER_FREQ/HZ) );
	out_byte(TIMER0, (uint8_t) ((TIMER_FREQ/HZ) >> 8));

	put_irq_handler(CLOCK_IRQ, clock_handler);
	enable_irq(CLOCK_IRQ);	
}

// priority is fixed value, ticks is counting down.
// when all processes ticks are 0, then reset ticks to it's priority.
void schedule(){
	struct proc* p;
	int greatest_ticks = 0;
	while(!greatest_ticks){
		for( p = proc_table; p < proc_table + NR_TASKS + NR_PROCS; p++)
			if(p->p_flags == 0){
				if(p->ticks > greatest_ticks){
					greatest_ticks = p->ticks;
					p_proc_ready = p;
				}
			}

		if(!greatest_ticks)
			for(p = proc_table; p < proc_table + NR_TASKS + NR_PROCS; p++)
				if (p->p_flags == 0)p->ticks = p->priority;
	}
}

void clock_handler(int irq){
	//kprint("[");

	ticks++;
	p_proc_ready->ticks--;

	if(k_reenter != 0){ // interrupt re-enter
		kprint(".");
		return;
	}

	//if (p_proc_ready->ticks > 0) return;
	schedule(); 
	//kprint("]");
}

// round robin version of scheduler
void clock_handler2(int irq){
	//kprint("[");

	ticks++;
	if(k_reenter != 0){
		//kprint("!");
		return;
	}

	// round robin process scheduler.
	p_proc_ready++;
	if(p_proc_ready >= proc_table + NR_TASKS + NR_PROCS)
		p_proc_ready = proc_table;

	//kprint("]");
}

