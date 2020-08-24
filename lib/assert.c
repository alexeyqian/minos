
#include "const.h"
#include "types.h"
#include "ke_asm_utils.h"
#include "stdio.h"

/*
PUBLIC void assertion_failure(char* exp, char* file, char* base_file, int line){
    printl("%c  assert(%s) failed. file: %s, base_file: %s, ln: %d",
        MAG_CH_ASSERT, exp, file, base_file, line);
    // if assertion fails in a task, the system will halt before printl returns.
    // if it happens in a user proccess, printl will return like a common routine.
    
    // use a for ever loop to prevent the proc from going on.
    spin("assertion_failure()");
    // should never arrive here
    //__asm__ __volatile("ud2");
}*/
