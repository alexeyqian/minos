#include "kernel.h"

int sys_kcall(int _unused1, int func, void* param, struct proc* p_proc){
    UNUSED(_unused1);
    char* str;
    void* buf;
    uint16_t* p_uint16;
    unsigned int* p_uint;
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
        case KC_ENABLE_INT:
            set_intr();
            break;
        case KC_DISABLE_INT:
            clear_intr();
            break;
        case KC_GET_CURSOR:
            return get_cursor();
            break;
        case KC_SET_CURSOR:
            p_uint = (unsigned int*)la_param;            
            set_cursor(*p_uint * 2); // convert position to offset (=2*position)
            break;
        case KC_SET_VIDEO_START_ADDR:
            p_uint = (unsigned int*)la_param; 
            //clear_intr(); // TODO: use some other atomic technics
            out_byte(CRTC_ADDR_REG, CRTC_DATA_IDX_START_ADDR_H);
            out_byte(CRTC_DATA_REG, (*p_uint >> 8) & 0xFF);
            out_byte(CRTC_ADDR_REG, CRTC_DATA_IDX_START_ADDR_L);
            out_byte(CRTC_DATA_REG, *p_uint & 0xFF);
            //set_intr();
            break;
        case KC_KEYBOARD_READ:
            return read_from_kb_buf();
            break;     
        case KC_SET_KEY_PRESSED:
            p_int = (int*)la_param;
            set_key_pressed(*p_int);
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