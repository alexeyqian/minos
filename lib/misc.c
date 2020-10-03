#include <const.h>
#include <sys/types.h>
#include <minos/const.h>
#include <minos/types.h>
#include <minos/proto.h>
#include <utils.h>

PUBLIC uint8_t inb(uint16_t port){
    return kcall(KC_IN_BYTE, &port);
}

PUBLIC void outb(uint16_t port, uint8_t value){
    struct two_ints_s param;
    param.i1 = port;
    param.i2 = value;
    kcall(KC_OUT_BYTE, &param);
}

void portread(uint16_t port, void* buf, int n){
    struct three_ints_s param;
    param.i1 = port;
    param.i2 = (int)buf;
    param.i3 = n;
    kcall(KC_PORT_READ, &param);
}

void portwrite(uint16_t port, void* buf, int n){
    struct three_ints_s param;
    param.i1 = port;
    param.i2 = (int)buf;
    param.i3 = n;
    kcall(KC_PORT_WRITE, &param);
}

void enableirq(int irq){
    kcall(KC_ENABLE_IRQ, &irq);
}

PUBLIC int kc_ticks(){
    return kcall(KC_TICKS, NULL);
}

PUBLIC void udelay(int milli_sec){
    int t = kc_ticks();
    while(((kc_ticks() - t) * 1000 / HZ) < milli_sec){}
}

PUBLIC void assertion_failurex(char* exp, char* file, char* base_file, int line){
    printx("!!kassert(%s)!! failed. file: %s, base_file: %s, ln: %d",
        exp, file, base_file, line);
    spinx("assertx failed!\n"); // halt()
}

PUBLIC void panicx(const char *fmt, ...)
{
	char buf[256];
	va_list arg = (va_list)((char*)&fmt + 4);
	vsprintf(buf, fmt, arg);
    printx("!!panicx!! %s", buf);
    spinx("panicx\n");//halt();
}

PUBLIC void spinx(char* func_name){
    printx(">>> spinx in %s ... \n", func_name);
    while(TRUE){}
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