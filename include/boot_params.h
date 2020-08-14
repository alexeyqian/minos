#ifndef MINOS_BOOT_PARAMS_H
#define MINOS_BOOT_PARAMS_H

struct boot_params;

void get_boot_params(struct boot_params* pbp);
void print_boot_params(struct boot_params* bp);
int get_kernel_map(unsigned int* base, unsigned int* limit, struct boot_params* bp);

#endif