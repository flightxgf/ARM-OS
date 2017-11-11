#ifndef __PROCESS_H
#define __PROCESS_H

// Include functionality relating to newlib (the standard C library).

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

//include functionality relating to the kernel

#include "scheduler.h"
#include "pcb.h"
#include "memory.h"
#include "pipe.h"


void init_processes();

void printCurrentPCB(ctx_t* ctx); 
void printPCB(pcb_t pcb);

int add_process(uint32_t exec_start);
void switch_process(ctx_t* ctx, int id);

int stdin_insert(char c);

extern pcb_t pcb[64];
extern int current_process;
extern int process_amount;
extern unsigned char dead_processes[64];

extern char STDIN[64][128];
extern int stdin_head[64];

#endif