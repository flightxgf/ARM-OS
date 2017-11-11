#ifndef __SCHEDULER_H
#define __SCHEDULER_H

// Include functionality relating to newlib (the standard C library).

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

//include functionality relating to the kernel

#include "process.h"
#include "pcb.h"

//Include functionality from standard libraries

#include "bin.h"

#define PRIORITY_REALTIME 5
#define PRIORITY_VERYHIGH 15
#define PRIORITY_HIGH     7
#define PRIORITY_NORMAL   3
#define PRIORITY_LOW      1
#define PRIORITY_NONE     0

void scheduler(ctx_t *ctx);
extern bin_heap* process_queue;
extern int scheduler_mode;

#endif