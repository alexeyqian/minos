#ifndef MINOS_KLIB_H
#define MINOS_KLIB_H
#include "const.h"
#include "types.h"
#include "ktypes.h"
#include "global.h"
#include "string.h"

#define proc2pid(x) (x - proc_table)
void* va2la(int pid, void*va);

/**
 * `phys_copy' and `phys_set' are used only in the kernel, where segments
 * are all flat (based on 0). In the meanwhile, currently linear address
 * space is mapped to the identical physical address space. Therefore,
 * a `physical copy' will be as same as a common copy, so does `phys_set'.
 */
#define	phys_copy	memcpy
#define	phys_set	memset

#endif