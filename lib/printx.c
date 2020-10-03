#include <const.h>
#include <sys/types.h>
#include <minos/const.h>
#include <minos/types.h>
#include <minos/proto.h>
#include <utils.h>

#define CHARBUF 256
//@attention used before tty is ready, after that, use printf
PUBLIC void printx(const char *fmt, ...){
    int i;
    char buf[CHARBUF];
    va_list args = (va_list)((char*)(&fmt) + 4); 
    // points to next params after fmt
    // now args is actually the addr of arg1 just behind fmt
    // args is actually a char*   
    i = vsprintf(buf, fmt, args);    
    buf[i] = 0;

    kcall(KC_PUTS, buf); 
}
