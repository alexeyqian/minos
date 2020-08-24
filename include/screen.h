#ifndef MINOS_SCREEN_H
#define MINOS_SCREEN_H

void kprintf(const char *fmt, ...);
void kclear_screen();

void kpanic(const char *fmt, ...); // use printl inside
void kspin(char* func_name);
void spin(char* func_name);       // use printl insede

void kassertion_failure(char *exp, char *file, char *base_file, int line);
#define kassert(exp)  if (exp) ; \
        else kassertion_failure(#exp, __FILE__, __BASE_FILE__, __LINE__)

#endif