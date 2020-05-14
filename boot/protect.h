#ifndef _MINOS_PROTECT_H_
#define _MINOS_PROTECT_H_
#include "type.h"

struct descriptor{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t attr;
    uint8_t limit_high_attr2;
    uint8_t base_high;
};

#endif