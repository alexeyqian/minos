#ifndef MINOS_KIO_H
#define MINOS_KIO_H


#define	O_CREAT		1
#define	O_RDWR		2

#define SEEK_SET	1
#define SEEK_CUR	2
#define SEEK_END	3

#define	MAX_PATH	128

int printf(const char *fmt, ...);
int sprintf(char* buf, const char* fmt, ...);
#define printl printf
void panic(const char *fmt, ...);
void spin(char* func_name);

#endif