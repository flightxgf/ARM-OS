#include "scheduler.h"

bin_heap *process_queue;
int scheduler_mode = 1;

void rr_scheduler(ctx_t *ctx) {

  int old_process = current_process;
  int new_process = (current_process + 1) % process_amount;
  while(dead_processes[new_process]){
    new_process = (new_process + 1) % process_amount;
  }
  // printf("Attempting to switch to process %d\n", new_process);
  
  switch_process(ctx, new_process);

  return;
}

void pq_scheduler(ctx_t *ctx){
  // heap_display(process_queue);
  pcb_t *next = (pcb_t*)(heap_pop(process_queue));
  for(int I = 0; I < process_queue->count; I++){
    pcb_t *proc = (pcb_t*)(process_queue->heaparr[I].data);
    proc->wait_time++;
    if (proc->priority == PRIORITY_REALTIME){
      process_queue->heaparr[I].priority = UINT32_MAX;
    }
    process_queue->heaparr[I].priority = proc->priority * proc->wait_time;
  }
  switch_process(ctx, next->pid);
  next->wait_time = 1;
  heap_push(process_queue, next, next->priority);
  for(int I = 0; I < process_queue->count;I++){
	  max_heapify(process_queue->heaparr, I, process_queue->count);
  }
  // heap_display(process_queue);
}

void scheduler(ctx_t *ctx){
  if (scheduler_mode){
    pq_scheduler(ctx);
  }
  else{
    rr_scheduler(ctx);
  }
}