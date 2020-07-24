#include "const.h"
#include "types.h"
#include "kio.h"

// <ring 1>
PUBLIC void task_fs(){
    printl(">>> task fs begins. \n");

    MESSAGE driver_msg;
    driver_msg.type = DEV_OPEN;
    send_recv(BOTH, TASK_HD, &driver_msg);
    spin("fs");
}