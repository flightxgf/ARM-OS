#ifndef __IRQ_HANDLER_H
#define __IRQ_HANDLER_H

// Include functionality relating to newlib (the standard C library).

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

//include functionality relating to the defice

#include   "PL011.h"
#include   "SP804.h"
#include     "GIC.h"

//include functionality relating to the kernel

#include "process.h"
#include "int.h"

#endif