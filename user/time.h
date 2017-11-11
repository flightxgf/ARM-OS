#ifndef __TIME_H
#define __TIME_H

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

#include "lolevel.h"
#include     "int.h"

void sleep(int x);
#endif

