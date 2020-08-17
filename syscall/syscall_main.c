#include "syscall.h"
#include "ipc.h"
#include "const.h"
#include "types.h"
#include "ktypes.h"
#include "global.h"
#include "assert.h"
#include "string.h"
#include "klib.h"
#include "kio.h"

PUBLIC int get_ticks(){
    MESSAGE msg;
    reset_msg(&msg);
    msg.type = GET_TICKS;
    send_recv(BOTH, TASK_SYS, &msg);
    return msg.RETVAL;
}

// <ring 1>, "system calls" via IPC message
PUBLIC void task_sys(){
    kspin("task_sys");
    kprintf(">>> 2. task_sys is running ... ...\n"); 
    MESSAGE msg;
    while(1){
        send_recv(RECEIVE , ANY, &msg);
        int src = msg.source;
        switch(msg.type){
            case GET_TICKS:
                msg.RETVAL = ticks;
                send_recv(SEND, src, &msg);
                break;
            case GET_PID:
                msg.type = SYSCALL_RET;
                msg.PID = src;
                send_recv(SEND, src, &msg);
                break;
            default:
                panic("unknown msg type");
                break;
        }
    }
}
