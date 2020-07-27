#include "fs.h"
#include "const.h"
#include "types.h"
#include "ktypes.h"
#include "string.h"
#include "kio.h"
#include "ipc.h"

// <ring 1>
PUBLIC void task_fs(){
    printl(">>> task fs begins. \n");

    MESSAGE driver_msg;
    driver_msg.type = DEV_OPEN;
    send_recv(BOTH, TASK_HD, &driver_msg);
    spin("fs");
}