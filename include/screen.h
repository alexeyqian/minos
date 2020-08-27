#ifndef MINOS_SCREEN_H
#define MINOS_SCREEN_H

void kprintf(const char *fmt, ...);
void kclear_screen();

void kpanic(const char *fmt, ...);
void kspin(char* func_name);
void spin(char* func_name);      

 // for kernel
void kassertion_failure(char *exp, char *file, char *base_file, int line);
#define kassert(exp)  if (exp) ; \
        else kassertion_failure(#exp, __FILE__, __BASE_FILE__, __LINE__)

 // for user
void assertion_failure(char *exp, char *file, char *base_file, int line);
#define assert(exp)  if (exp) ; \
        else assertion_failure(#exp, __FILE__, __BASE_FILE__, __LINE__)

void never_here();
#endif