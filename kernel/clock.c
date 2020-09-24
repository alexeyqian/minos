#include "clock.h"
#include "types.h"
#include "ktypes.h"
#include "global.h"
#include "ke_asm_utils.h"
#include "klib.h"
#include "screen.h"
#include "ipc.h"
#include "syscall.h"
#include "interrupt.h"

// TODO: update as PRIVATE
PUBLIC void init_clock(){ // init 8253 PIT
	out_byte(TIMER_MODE, RATE_GENERATOR);
	out_byte(TIMER0, (uint8_t) (TIMER_FREQ/HZ) );
	out_byte(TIMER0, (uint8_t) ((TIMER_FREQ/HZ) >> 8));

	put_irq_handler(CLOCK_IRQ, clock_handler);
	enable_irq(CLOCK_IRQ);	
}
/*
PUBLIC void clock_task(){	
	MESSAGE msg; // message buffer for both input and output
	int result;

	init_clock();

	// process request, nver reply
	while(TRUE){
		send_recv(RECEIVE, ANY, &msg);  
		switch(msg.type){
			case HARD_INT:
				result = do_clocitick(&msg);
				break;
			default:
				kprintf("CLOCK: illegal request. Type: %d, from %d.\n", m.type, m.source);
		}
	}

}*/

PUBLIC void delay(int milli_sec){
    int t = get_ticks();
    while(((get_ticks() - t) * 1000 / HZ) < milli_sec) {}
}

PRIVATE void clock_handler(int irq){
	UNUSED(irq);
	
	if(++ticks >= MAX_TICKS) ticks = 0;

	if(p_proc_ready->ticks)
		p_proc_ready->ticks--;

	if(key_pressed)
		inform_int(TASK_TTY);
		
	if(k_reenter != 0){ // interrupt re-enter
		return;
	}

	if (p_proc_ready->ticks > 0) return;
	schedule(); 
}



// priority is fixed value, ticks is counting down.
// when all processes ticks are 0, then reset ticks to it's priority.
PUBLIC void schedule(){
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
	if(p_proc_ready >= proc_table + NR_TASKS + NR_PROCS)
		p_proc_ready = proc_table;
}*/

