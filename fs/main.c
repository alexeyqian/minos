#include "fs.h"
#include "const.h"
#include "types.h"
#include "ktypes.h"
#include "string.h"
#include "global.h"
#include "assert.h"
#include "kio.h"
#include "ipc.h"

// 6M-7M buffer for fs
//PRIVATE uint8_t fsbuf = (uint8_t*)0x600000;
//PRIVATE const int FSBUF_SIZE = 0x100000;

// <ring 1>
PUBLIC void task_fs(){
    printl(">>> task fs begins. \n");

    MESSAGE driver_msg;
    driver_msg.type = DEV_OPEN;
    driver_msg.DEVICE = MINOR(ROOT_DEV);
    assert(dd_map[MAJOR(ROOT_DEV)].driver_nr != INVALID_DRIVER);
    send_recv(BOTH, dd_map[MAJOR(ROOT_DEV)].driver_nr, &driver_msg);
    spin("fs");
}