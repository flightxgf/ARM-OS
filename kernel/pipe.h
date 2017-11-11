#ifndef __PIPE_H
#define __PIPE_H

// Include functionality relating to newlib (the standard C library).

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

//include functionality relating to the kernel


typedef struct {
  uint32_t host;
  uint32_t client;
  uint32_t head;
  uint8_t  data[1048564];
} pipe_t;


#endif