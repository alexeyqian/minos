#ifndef MINOS_TYPES_H
#define MINOS_TYPES_H

typedef int              bool_t;
typedef char             int8_t;
typedef unsigned char    uint8_t;
typedef short            int16_t;
typedef unsigned short   uint16_t;
typedef int              int32_t;
typedef unsigned int     uint32_t;
typedef unsigned long long uint64_t;

typedef int32_t          intptr_t;
typedef uint32_t         uintptr_t;

typedef uint32_t         size_t;
typedef int32_t          ssize_t;
typedef int32_t          off_t;

typedef char*			va_list;

typedef unsigned short io_port_t;
typedef void (*pf_int_handler_t)();
typedef void (*pf_irq_handler_t)(int irq);
typedef void (*pf_task_t)();
typedef void* syscall_t;

#endif