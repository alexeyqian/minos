#ifndef KERNEL_H
#define KERNEL_H

// POSIX headers
#include <const.h>
#include <sys/types.h>
#include <limits.h>
#include <string.h>
#include <utils.h>
#include <elf.h>
#include <stdio.h>

// MINOS specific headers
#include <minos/const.h>
#include <minos/types.h>
#include <minos/proto.h>
#include <minos/fs.h>
#include <minos/pm.h>
#include <minos/keyboard.h>
#include <minos/tty.h>

// MINOS kernel headers
#include "ke_asm_utils.h"   // functions have to be written in asm
#include "kconst.h"
#include "ktypes.h"
#include "proto.h"          // function prototypes
#include "global.h"
#endif