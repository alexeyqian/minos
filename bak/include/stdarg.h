#ifndef STDARG_H
#define STDART_H

typedef char *va_list;

#define __va_promote(type) \
    (((sizeof(type) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))
    
#define va_start(ap, last) \
    (ap = ((char*)&(last) + __va_promote(last)))
    
#define var_arg(ap, type) \ 
    ((type *)(ap += sizeof(type)))[-1]
    
#define va_end(ap)

#endif