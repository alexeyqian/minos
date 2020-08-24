#ifndef MINOS_KIO_H
#define MINOS_KIO_H

#define	STR_DEFAULT_LEN	1024
#define	MAX_PATH	128

#define	O_CREAT		1
#define	O_RDWR		2

#define SEEK_SET	1
#define SEEK_CUR	2
#define SEEK_END	3

int sprintf(char* buf, const char* fmt, ...); // print to buf
//int printl(const char *fmt, ...);             // low level print, for kernel/tasks
// user process print, cannot use in kernel/tasks
// prerequisite: tty has already been opened and with fd = 1
int printf(const char *fmt, ...);             

#endif