#ifndef __RST_HANDLER_H
#define __RST_HANDLER_H

// Include functionality relating to newlib (the standard C library).

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

//include functionality relating to the defice

#include   "PL011.h"
#include   "SP804.h"
#include   "GIC.h"
#include   "MMU.h"
#include   "int.h"
#include   "disk.h"

//include functionality relating to the kernel

#include "process.h"
#include "memory.h"
#include "filesystem.h"

//Include functionality from standard libraries

#include "io.h"

#endif