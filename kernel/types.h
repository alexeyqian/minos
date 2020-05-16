#ifndef _MINOS_TYPES_H_
#define _MINOS_TYPES_H_

#define TRUE  1
#define FALSE 0

typedef int              bool_t;
typedef char             int8_t;
typedef unsigned char    uint8_t;
typedef short            int16_t;
typedef unsigned short   uint16_t;
typedef int              int32_t;
typedef unsigned int     uint32_t;

typedef int32_t          intptr_t;
typedef uint32_t         uintptr_t;

typedef uint32_t         size_t;
typedef int32_t          ssize_t;
typedef int32_t          off_t;

typedef unsigned short io_port_t;
typedef void (*pf_int_handler_t)();

struct descriptor{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t attr;
    uint8_t limit_high_attr2;
    uint8_t base_high;
};

struct gate{
    uint16_t offset_low;
    uint16_t selector;    
    // only used in call gate, number of parameters 
    // which need to be copied to kernel stack
    uint8_t  dcount;      
    uint8_t  attr;        // P(1)DPL(2)DT(1)TYPE(4)
    uint16_t offset_high;
};

#endif