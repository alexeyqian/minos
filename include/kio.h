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
int printl(const char *fmt, ...);             // low level print
int printf(const char *fmt, ...);             // user process print

void panic(const char *fmt, ...);
void spin(char* func_name);

#endif