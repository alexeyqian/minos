#ifndef MINOS_KIO_H
#define MINOS_KIO_H

int printf(const char *fmt, ...);
int sprintf(char* buf, const char* fmt, ...);
#define printl printf
void panic(const char *fmt, ...);
void spin(char* func_name);

#endif