#include "stdio.h"

#include "const.h"
#include "types.h"
#include "vsprintf.h"
#include "ke_asm_utils.h"

#include "fs.h"

/**
 * Low level print
 * 
 * @return the number of chars printed.
 * */
/*
PUBLIC int printl(const char* fmt, ...){
    int i;
    char buf[STR_DEFAULT_LEN];
    va_list arg = (va_list)((char*)(&fmt) + 4);
    i = vsprintf(buf, fmt, arg);
    printx(buf);
    return i;
}*/

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

/**
 * Write formated string tu buf
 * 
 * @param buf formated string will be written here
 * */
PUBLIC int sprintf(char* buf, const char* fmt, ...){
    va_list arg = (va_list)((char *)(&fmt) + 4);
    return vsprintf(buf, fmt, arg);
}

PUBLIC void spin(char* func_name){
        //printl(">>> spinning in %s ... \n", func_name);
    printf(">>> kspinning in %s ... \n", func_name);
    while(1){}
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

