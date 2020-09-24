#ifndef _ERRNO_H
#define _ERRNO_H

#ifdef _SYSTEM
#    define _SIGN   -
#    define OK      0
#else
#    define _SIGN
#endif

extern int errno;

#define _NERROR   70 // number of errors

#define EGENERIC (_SIGN 99)
#define EPERM    (_SIGN  1)  // operation not permitted
#define ENOENT   (_SIGN  2)  // no such file or directory


#define ECALLDENIED (_SIGN 104) // no permissions for system call

#define EBADREQUEST (_SIGN 107) // destination cannot handle request

#define EDONTREPLY  (_SIGN 201) // pseudo code, don't send a reply
#endif