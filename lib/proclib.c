#include "proc.h"
#include "const.h"
#include "types.h"
#include "ktypes.h"

#include "ipc.h"
#include "stdio.h"
#include "string.h"
#include "screen.h"

PUBLIC int getpid(){
	MESSAGE msg;
	msg.type = GET_PID;
	send_recv(BOTH, TASK_SYS, &msg);
	return msg.PID;
}

// terminate the current process
// @param status the value returned to the parent
PUBLIC void exit(int status){
	MESSAGE msg;
	msg.type = EXIT;
	msg.STATUS = status;
	send_recv(BOTH, TASK_MM, &msg);
	assert(msg.type == SYSCALL_RET);
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
    assert(msg.type == SYSCALL_RET);
    assert(msg.RETVAL == 0);

    return msg.PID;
}

PUBLIC int wait(int* status){
	MESSAGE msg;
	msg.type = WAIT;
	send_recv(BOTH, TASK_MM, &msg);
	*status = msg.STATUS;
	return (msg.PID == NO_TASK ? -1 : msg.PID);
}

PUBLIC int execl(const char* path, const char *arg, ...){
	va_list parg = (va_list)(&arg);
	char** p = (char**)parg;
	return execv(path, p);
}

PUBLIC int execv(const char* path, char* argv[]){
	char** p = argv;
	char arg_stack[PROC_ORIGIN_STACK];
	uint32_t stack_len = 0;

	while(*p++){
		assert(stack_len + 2*sizeof(char*) < PROC_ORIGIN_STACK);
		stack_len += sizeof(char*);
	}

	*((int*)(&arg_stack[stack_len])) = 0; // set 0 to pointers array
	stack_len += sizeof(char*);

	// loop all args, copy them to arg stack array
	// set correct pointer for each string
	char** q = (char**)arg_stack;
	for(p = argv; *p != 0; p++){
		*q++ = &arg_stack[stack_len]; // set address of arg to pointer array item
		assert(stack_len + strlen(*p) + 1 < PROC_ORIGIN_STACK);
		strcpy(&arg_stack[stack_len], *p);
		stack_len += strlen(*p);
		arg_stack[stack_len] = 0; // set zero as string end for arg
		stack_len++;
	}

	MESSAGE msg;
	msg.type = EXEC;
	msg.PATHNAME = (void*)path;
	msg.NAME_LEN = strlen(path);
	msg.BUF = (void*)arg_stack;
	msg.BUF_LEN = stack_len;

	send_recv(BOTH, TASK_MM, &msg);
	assert(msg.type == SYSCALL_RET);

	return msg.RETVAL;
}
