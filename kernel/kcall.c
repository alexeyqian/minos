#include "kernel.h"

int sys_kcall(int _unused1, int func, void* param, struct proc* p_proc){
    UNUSED(_unused1);
    char* str;
    
    //kprintf("call in sys_kcall, func: %d.\n", func);
    switch(func){
        case KC_PUTS:
            str = (char*)va2la(proc2pid(p_proc), param);
            kputs(str);
            break;
        case KC_TICKS:
            return ticks;
            break;
        default:
            kpanic("sys_kcall: unknown func: %d", func);
            break;
    }
    return OK;
}