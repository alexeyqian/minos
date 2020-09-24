#include "system.h"
#include "ipc.h"
#include "const.h"
#include "types.h"
#include "ktypes.h"
#include "global.h"

#include "string.h"
#include "klib.h"
#include "stdio.h"

PRIVATE void init(){
    struct priv *sp;
    int i;

    // init IRQ handler hooks.
    // Mark all hooks available.
    for(i = 0; i < NR_IRQ_HOOKS; i++){
        irq_hooks[i].proc_nr = NONE;
    }

    // init all alarm timers for all processes.
    for(sp = BEG_PRIV_ADDR; sp < END_PRIV_ADDR; sp++){
        tmr_inittimer(&(sp->s_alarm_timer));
    }
}

PUBLIC void sys_task(){
    static MESSAGE msg;
    int result;
    struct proc *caller_ptr;
    unsigned int call_nr;
    int status;

    init();

    while(TRUE){
        send_recv(RECEIVE, ANY, &msg);
        call_nr = (unsigned)msg.type - KERNEL_CALL;
        caller_ptr = proc_addr(msg.source);

        // see if the caller made a valid request and try to handle it.
        if(! (priv(caller_ptr)->s_call_mask & (1 << call_nr))){
            kprintf("SYSTEM: request %d from %d denied.\n", call_nr, msg.source);
            result = ECALLDENIED;
        }else if(call_nr >= NR_SYS_CALLS){
            kprintf("SYSTEM: illegal request %d from %d denied.\n", call_nr, msg.source);
            result = EBADREQUEST;
        }else{
            result = (*call_vec[call_nr])(&msg);
        }

        // Send a reply, unless inhibited by a handler function.
        // Use the kernel function lock_send() to prevent a system call trap.
        // The destination is known to be blocked waiting for a message.

        if(result != EDONOTREPLY){
            msg.type = result;
            if(OK != (status = lock_send(msg.source, &msg))){
                kprintf("SYSTEM, reply to %d failed: %d.\n", msg.source , status);
            }
        }
    }
}