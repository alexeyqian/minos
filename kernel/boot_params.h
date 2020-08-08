#ifndef MINOS_BOOT_PARAMS_H
#define MINOS_BOOT_PARAMS_H
#include "ktypes.h"
void get_boot_params(struct boot_params* pbp);
void print_boot_params(struct boot_params* bp);

#endif