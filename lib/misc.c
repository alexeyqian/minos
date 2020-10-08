#include <const.h>
#include <sys/types.h>
#include <minos/const.h>
#include <minos/types.h>
#include <minos/proto.h>
#include <minos/fs.h>
#include <utils.h>
#include <stdio.h>

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

void setintr(){
    kcall(KC_ENABLE_INT, NULL);
}

void clearintr(){
    kcall(KC_DISABLE_INT, NULL);
}

unsigned int getcursor(){
    return kcall(KC_GET_CURSOR, NULL);
}

void setcursor(unsigned int position){
    kcall(KC_SET_CURSOR, &position);
}

void setvideostartaddr(unsigned int addr){
    kcall(KC_SET_VIDEO_START_ADDR, &addr);
}

PUBLIC uint8_t retrive_scan_code_from_kb_buf(){
    return (uint8_t)kcall(KC_KEYBOARD_READ, NULL);
}

PUBLIC void setkeypressed(int value){
    kcall(KC_SET_KEY_PRESSED, &value);
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

PUBLIC void assertion_failure(char* exp, char* file, char* base_file, int line){
    printf("!!kassert(%s)!! failed. file: %s, base_file: %s, ln: %d",
        exp, file, base_file, line);
    spin("assertx failed!\n"); // halt()
}

PUBLIC void panic(const char *fmt, ...)
{
	char buf[256];
	va_list arg = (va_list)((char*)&fmt + 4);
	vsprintf(buf, fmt, arg);
    printf("!!panic!! %s", buf);
    while(TRUE){}
}

PUBLIC void spin(char* func_name){
    printf(">>> spinx in %s ... \n", func_name);
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


PUBLIC void reset_msg(KMESSAGE* p)
{
	memset(p, 0, sizeof(KMESSAGE));
}

/**
 * Write formated string tu buf
 * 
 * @param buf formated string will be written here
 * */
PUBLIC int sprintf(char* buf, const char* fmt, ...){
    va_list arg = (va_list)((char *)(&fmt) + 4);
    return vsprintf(buf, fmt, arg);
}

/**
 * User space print, cannot be used in kernel or tasks.
 * make sure the caller process has already opened console file, and set to 1.
 * C calling convension is caller clear the params in stack
 *  since for this type of variable params,
 *  only caller knows how many params used.
 * 
 * @return the number of chars printed
 * */
PUBLIC int printf(const char *fmt, ...){
    char buf[STR_DEFAULT_LEN];
    va_list args = (va_list)((char*)(&fmt) + 4); 
    int i = vsprintf(buf, fmt, args);     
    write(FD_STDOUT, buf, i);    
    return i;
}


/******************************************************************************************
                                     Example
===========================================================================================

i = 0x23;
j = 0x78;
char fmt[] = "%x%d";
printf(fmt, i, j);

        push    j
        push    i
        push    fmt
        call    printf
        add     esp, 3 * 4


                ┃   HIGH   ┃                        ┃   HIGH   ┃
                ┃   ...    ┃                        ┃   ...    ┃
                ┣━━━━━━━━━━┫                        ┣━━━━━━━━━━┫
                ┃          ┃                 0x32010┃   '\0'   ┃
                ┣━━━━━━━━━━┫                        ┣━━━━━━━━━━┫
         0x3046C┃   0x78   ┃                 0x3200c┃    d     ┃
                ┣━━━━━━━━━━┫                        ┣━━━━━━━━━━┫
   arg = 0x30468┃   0x23   ┃                 0x32008┃    %     ┃
                ┣━━━━━━━━━━┫                        ┣━━━━━━━━━━┫
         0x30464┃ 0x32000  ╂───--------─┐    0x32004┃    x     ┃
                ┣━━━━━━━━━━┫            │           ┣━━━━━━━━━━┫
                ┃          ┃            └──→ 0x32000┃    %     ┃
                ┣━━━━━━━━━━┫                        ┣━━━━━━━━━━┫
                ┃    ...   ┃                        ┃   ...    ┃
                ┃    LOW   ┃                        ┃   LOW    ┃

    here is how vsprintf get called.
    vsprintf(buf, 0x32000, 0x30468);
*/

