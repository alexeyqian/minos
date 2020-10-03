#include "kernel.h"

int sys_kcall(int _unused1, int func, void* param, struct proc* p_proc){
    UNUSED(_unused1);
    char* str;
    void* buf;
    uint16_t* p_uint16;
    int* p_int;
    struct two_ints_s* two_ints;
    struct three_ints_s* three_ints;
    void* la_param;
    if(param != NULL)
        la_param = va2la(proc2pid(p_proc), param);

    //kprintf("call in sys_kcall, func: %d.\n", func);
    switch(func){
        case KC_IN_BYTE:
            p_uint16 = (uint16_t*)la_param;
            return in_byte(*p_uint16);
            break;
        case KC_OUT_BYTE:
            two_ints = (struct two_ints_s*)la_param;
            out_byte((uint16_t)(two_ints->i1), (uint8_t)(two_ints->i2));
            break;
        case KC_PORT_READ:
            three_ints = (struct three_ints_s*)la_param;
            buf =  va2la(proc2pid(p_proc), (void*)(three_ints->i2));
            port_read((uint16_t)(three_ints->i1), buf ,(three_ints->i3));            
            break;
        case KC_PORT_WRITE:
            three_ints = (struct three_ints_s*)la_param;
            buf =  va2la(proc2pid(p_proc), (void*)(three_ints->i2));
            port_write((uint16_t)(three_ints->i1), buf ,(three_ints->i3));            
            break;
        case KC_ENABLE_IRQ:
            p_int = (int*)la_param;
            enable_irq(*p_int);
            break;
        case KC_PUTS:
            str = (char*)la_param;
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