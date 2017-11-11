#ifndef __BIN_H
#define __BIN_H

// Include functionality relating to newlib (the standard C library).

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

typedef struct {
  int priority;
  void *data;
} heap_node;

typedef struct {
	int size;
	int count;
	heap_node *heaparr;
} bin_heap;

void heap_init(bin_heap *h);
void* heap_pop(bin_heap *h);
void heap_display(bin_heap *h);
void heap_remove(bin_heap *h, void *value);
void heap_push(bin_heap *h, void *value, int priority);

#endif