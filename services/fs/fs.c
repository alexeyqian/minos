#include "_fs.h"
#include <minos/fs.h>

PRIVATE void test_open_hd(){
    KMESSAGE driver_msg;
    driver_msg.type = DEV_OPEN;
    driver_msg.DEVICE = MINOR(ROOT_DEV);
    //send_recv(BOTH, get_dev_driver(ROOT_DEV), &driver_msg);
    send_recv(BOTH, TASK_HD, &driver_msg);
}

PUBLIC void svc_fs(){
    printx(">>> svc_fs is running.\n");
    //for(int i = 0; i < 3; i++)
    //    printx(">>> get ticks: %d\n", kc_ticks());

    test_open_hd();
    while(TRUE){}
}