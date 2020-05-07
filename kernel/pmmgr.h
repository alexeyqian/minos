#ifndef PHMGR_H
#define PHMGR_H
#include "types.h"

typedef uint32_t physical_addr;

struct mem_region{
    uint32_t base;
    uint32_t size;
};

void pmmgr_init(uint32_t mem_size_kb, uint32_t bitmap);

#endif