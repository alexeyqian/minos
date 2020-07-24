#ifndef MINOS_VSPRINTF_H
#define MINOS_VSPRINTF_H

#include "const.h"
#include "types.h"

int vsprintf(char *buf, const char *fmt, va_list args);

#endif