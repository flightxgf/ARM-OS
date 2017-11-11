#ifndef __PCB_H
#define __PCB_H

#include "pipe.h"

typedef struct {
  uint32_t cpsr, pc, gpr[ 13 ], sp, lr;
} ctx_t;

typedef struct {
  uint32_t head;
  uint32_t id;
  uint32_t ispipe;
  uint32_t isIO;
  pipe_t   *pipe;
  uint32_t mem_addr;
  uint32_t size;
  uint8_t  ro;
} file_descriptor;

typedef struct {
  pid_t pid;
  ctx_t ctx;
  uint32_t stack_top;
  uint32_t stack_current;
  uint32_t heap_current;
  uint32_t heap_bottom;
  uint32_t priority;
  uint32_t wait_time;
  uint32_t fd_amount;
  uint32_t pipe_amount;
  pipe_t   *pipes;
  file_descriptor     *fd_table;
 } pcb_t;


#endif
