#ifndef MINOS_ASSERT_H
#define MINOS_ASSERT_H

void kassertion_failure(char *exp, char *file, char *base_file, int line);
#define kassert(exp)  if (exp) ; \
        else kassertion_failure(#exp, __FILE__, __BASE_FILE__, __LINE__)

#endif