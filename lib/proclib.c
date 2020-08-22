#include "proc.h"
#include "const.h"
#include "types.h"
#include "ktypes.h"
#include "assert.h"
#include "ipc.h"

PUBLIC int getpid(){
	MESSAGE msg;
	msg.type = GET_PID;
	send_recv(BOTH, TASK_SYS, &msg);
	kassert(msg.type == SYSCALL_RET);
	return msg.PID;
}

// terminate the current process
// @param status the value returned to the parent
PUBLIC void exit(int status){
	MESSAGE msg;
	msg.type = EXIT;
	msg.STATUS = status;
	send_recv(BOTH, TASK_MM, &msg);
	kassert(msg.type == SYSCALL_RET);
}

/**
 * Create a child process, which is actually a copy of the caller.
 * 
 * @return   On success, the PID of the child process is returned in the
 *         parent's thread of execution, and a 0 is returned in the child's
 *         thread of execution.
 *           On failure, a -1 will be returned in the parent's context, no
 *         child process will be created.
 *****************************************************************************/

PUBLIC int fork(){
    MESSAGE msg;
    msg.type = FORK;
    send_recv(BOTH, TASK_MM, &msg);
    kassert(msg.type == SYSCALL_RET); // TODO: replace kassert with assert
    kassert(msg.RETVAL == 0);

    return msg.PID;
}

PUBLIC int wait(int* status){
	MESSAGE msg;
	msg.type = WAIT;
	send_recv(BOTH, TASK_MM, &msg);
	*status = msg.STATUS;
	return (msg.PID == NO_TASK ? -1 : msg.PID);
}

