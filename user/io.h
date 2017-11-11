#ifndef __IO_H
#define __IO_H

// Include functionality relating to newlib (the standard C library).

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <math.h>

// Include functionality relating to the platform.

#include   "GIC.h"
#include "PL011.h"

// Include functionality relating to the   kernel.

#include    "libc.h"
#include    "time.h"

void working_printf(char* str, ...);
void get_input(char *str, int n);
#endif

