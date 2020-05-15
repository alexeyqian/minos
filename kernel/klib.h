#ifndef _MINOS_KLIB_H_
#define _MINOS_KLIB_H_

#define PUBLIC
#define PRIVATE static // limit scope
#define GDT_SIZE 128 // number of descriptors in GDT/LDT 

typedef unsigned int   uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char  uint8_t;
typedef int            bool_t;

struct descriptor{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t attr;
    uint8_t limit_high_attr2;
    uint8_t base_high;
};

#endif