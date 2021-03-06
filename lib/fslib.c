
// POSIX headers
#include <const.h>
#include <sys/types.h>
#include <limits.h>
#include <string.h>
#include <utils.h>

// MINOS specific headers
#include <minos/const.h>
#include <minos/types.h>
#include <minos/proto.h>
#include <minos/fs.h>

/** 
 * open/create a file
 * @attention cannot use printf/assert inside, since the stdout might not open yet.
 * 
 * @param flag O_CREATE, O_RDWR
 * 
 * @return file descriptor if successful, otherwise -1
 */
PUBLIC int open(const char* pathname, int flags){
    KMESSAGE msg;
    msg.type = OPEN;
    msg.PATHNAME = (void*)pathname;
    msg.FLAGS = flags;
    msg.NAME_LEN = (int)strlen(pathname);
    send_recv(BOTH, TASK_FS, &msg);

    return msg.FD;
}


/**
 * Close a file descriptor.
 * 
 * @param fd  File descriptor.
 * 
 * @return Zero if successful, otherwise -1.
 *****************************************************************************/
PUBLIC int close(int fd)
{   
	KMESSAGE msg;
	msg.type   = CLOSE;
	msg.FD     = fd;
	send_recv(BOTH, TASK_FS, &msg);
	return msg.RETVAL;
}


/**
 * Read from a file descriptor
 * 
 * @param buf buffer to accept the bytes read
 * 
 * @return on success, the number of bytes read are returned
 *         on error, -1 is returned.
 * */
PUBLIC int read(int fd, void* buf, int count){
    KMESSAGE msg;
    msg.type = READ;
    msg.FD = fd;
    msg.BUF = buf;
    msg.CNT = count;

    send_recv(BOTH, TASK_FS, &msg);

    return msg.CNT;
}

/**
 * Delete a file
 * 
 * @return zero if successful, otherwise -1
 * */
PUBLIC int unlink(const char* pathname){
    KMESSAGE msg;
    msg.type = UNLINK;
    msg.PATHNAME = (void*)pathname;
    msg.NAME_LEN = (int)strlen(pathname);
    send_recv(BOTH, TASK_FS, &msg);
    return msg.RETVAL;
}


/**
 * Write to a file descriptor
 * 
 * @param buf buffer including the bytes to write
 * 
 * @return on success, the number of bytes written are returned
 *         on error, -1 is returned.
 * */
PUBLIC int write(int fd, const void* buf, int count){
    KMESSAGE msg;
    msg.type = WRITE;
    msg.FD = fd;
    msg.BUF = (void*)buf;
    msg.CNT = count;
    send_recv(BOTH, TASK_FS, &msg);

    return msg.CNT;
}

PUBLIC int stat(const char* path, struct stat* buf){
    KMESSAGE msg;
    msg.type = STAT;
    msg.PATHNAME = (void*)path;
    msg.BUF      = (void*)buf;
    msg.NAME_LEN = (int)strlen(path);
    send_recv(BOTH, TASK_FS, &msg);
    return msg.RETVAL;
}