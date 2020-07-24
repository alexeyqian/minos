#ifndef MINOS_ASSERT_H
#define MINOS_ASSERT_H

void assertion_failure(char* exp, char* file, char* base_file, int line);
#define assert(exp) if(exp);else assertion_failure(#exp, __FILE__, __BASE_FILE__, __LINE__)

#endif