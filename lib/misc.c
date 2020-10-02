#include <const.h>
#include <sys/types.h>
#include <minos/const.h>
#include <minos/types.h>
#include <minos/proto.h>
#include <utils.h>

PUBLIC int kc_ticks(){
    return kcall(KC_TICKS, NULL);
}

PUBLIC void udelay(int milli_sec){
    int t = kc_ticks();
    while(((kc_ticks() - t) * 1000 / HZ) < milli_sec){}
}

// wrapper for system call sendrec
// use this, diret call to sendrec should be avoided.
// always return 0;
PUBLIC int send_recv(int function, int src_dest, KMESSAGE* p_msg){
    int ret = 0;

    if(function == RECEIVE)
        memset((char*)p_msg, 0, sizeof(KMESSAGE));

    switch(function){
        case BOTH:
            ret = sendrec(SEND, src_dest, p_msg);
            if(ret == 0)
                ret = sendrec(RECEIVE, src_dest, p_msg);
            break;
        case SEND:
        case RECEIVE: 
            ret = sendrec(function, src_dest, p_msg);
            break;
        default:
            //assert((function == BOTH) || 
            //    (function == SEND) || (function == RECEIVE));
            break;
    }

    return ret;
}