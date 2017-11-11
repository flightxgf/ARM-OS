#ifndef __MEMORY_H
#define __MEMORY_H

// Include functionality relating to newlib (the standard C library).

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//include functionality relating to the defice

#include   "MMU.h"

//include functionality relating to the kernel

#include "process.h"



extern uint32_t kernel_heap;
extern uint32_t current_page_table [4096] __attribute__ ((aligned (1 << 14)));

void setup_mmu();
void create_page_table(int pid);
int allocate_new_page(int pid, uint32_t addr);
void fork_copy_ro(int pid, int new_pid);
void handle_data_abort(int pid, uint32_t addr);

#endif