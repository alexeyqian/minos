#include "ipc.h"
#include "const.h"
#include "types.h"
#include "ktypes.h"
#include "assert.h"

// ring 1-3, a wrapper for system call sendrec
// use this, diret call to sendrec should be avoided.
// always return 0;
PUBLIC int send_recv(int function, int src_dest, MESSAGE* p_msg){
    int ret = 0;

    if(function == RECEIVE)
        memset((char*)p_msg, 0, sizeof(MESSAGE));

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
            assert((function == BOTH) || 
                (function == SEND) || (function == RECEIVE));
            break;
    }

    return ret;
}
