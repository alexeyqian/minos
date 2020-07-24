#include "kio.h"

#include "const.h"
#include "types.h"
#include "vsprintf.h"
#include "ke_asm_utils.h"

// C calling convension is caller clear the params in stack
// since for this type of variable params,
// only caller knows how many params used.
PUBLIC int printf(const char *fmt, ...){
    int i;
    char buf[256];
    va_list args = (va_list)((char*)(&fmt) + 4); // points to next params after fmt
    // now args is actually the addr of arg1 just behind fmt
    // args is actually a char*
    i = vsprintf(buf, fmt, args); 
    buf[i] = 0;
    //write(buf, i);
	printx(buf); // syscall
    return i;
}

// TODO: create printfx as printl/printlx for assert / panic only
PUBLIC int sprintf(char* buf, const char* fmt, ...){
    va_list arg = (va_list)((char *)(&fmt) + 4);
    return vsprintf(buf, fmt, arg);
}

PUBLIC void spin(char* func_name){
    printl(">>> spinning in %s ... \n", func_name);
    while(1){}
}

PUBLIC void panic(const char *fmt, ...)
{
	int i;
	char buf[256];

	/* 4 is the size of fmt in the stack */
	va_list arg = (va_list)((char*)&fmt + 4);

	i = vsprintf(buf, fmt, arg);

	printl("%c !!panic!! %s", MAG_CH_PANIC, buf);

	/* should never arrive here */
	//__asm__ __volatile__("ud2");
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

