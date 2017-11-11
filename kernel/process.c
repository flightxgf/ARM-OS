#include "process.h"

pcb_t pcb[ 64 ];
char STDIN[64][128];
int stdin_head[64];
unsigned char dead_processes[64];
int   current_process = 0;
int   focused_process = 0;
int   process_amount = 0;

extern uint32_t tos_usr;

void init_processes(){
  current_process = 0;
  process_amount = 0;
}

void printCurrentPCB(ctx_t* ctx){ 
  printf("PROCESS\t%d\n", current_process); 
  printf ("CPSR:\t%08x\n", ctx->cpsr); 
  printf ("PC:\t%08x\n", ctx->pc); 
  for (int I = 0; I < 13; I++){ 
    printf("R%d:\t%08x\n", I, ctx->gpr[I]); 
  } 
  printf ("SP:\t%08x\n", ctx->sp); 
  printf ("LR:\t%08x\n", ctx->lr); 
} 

void printPCB(pcb_t pcb){ 
  printf("PROCESS\t%d\n", pcb.pid); 
  printf ("CPSR:\t%08x\n", pcb.ctx.cpsr); 
  printf ("PC:\t%08x\n", pcb.ctx.pc); 
  for (int I = 0; I < 13; I++){ 
    printf("R%d:\t%08x\n", I, pcb.ctx.gpr[I]); 
  } 
  printf ("SP:\t%08x\n", pcb.ctx.sp); 
  printf ("LR:\t%08x\n", pcb.ctx.lr); 
} 

int add_process(uint32_t exec_start){
  int new_id = -1;

  for (int I = 0; I < 64; I++){
    if(dead_processes[I]){
      new_id = I;
      dead_processes[I] = 0;
      process_amount++;
      break;
    }
  }
  if (new_id == -1){
    new_id = process_amount;
    process_amount++;
  }

  memset(&(pcb[new_id].ctx), 0, sizeof( ctx_t ) );
  pcb[new_id].pid           = new_id;
  pcb[new_id].priority      = PRIORITY_NORMAL;
  pcb[new_id].stack_top     = 0x8CEEF000;
  pcb[new_id].stack_current = pcb[new_id].stack_top;
  pcb[new_id].heap_bottom   = 0x70000000;
  pcb[new_id].heap_current  = pcb[new_id].heap_bottom;
  pcb[new_id].wait_time     = 0;
  pcb[new_id].fd_amount     = 3;
  pcb[new_id].pipes         = (pipe_t *)0xa0000000;
  pcb[new_id].fd_table      = malloc(3 * sizeof(file_descriptor));
  pcb[new_id].ctx.cpsr      = 0x50;
  pcb[new_id].ctx.pc        = (uint32_t)(exec_start );
  pcb[new_id].ctx.sp        = pcb[new_id].stack_top;

  memset(pcb[new_id].fd_table, 0, 3 * sizeof(file_descriptor));
  for (int I = 0; I < 3; I++){
    file_descriptor fd;
    fd.head   = 0;
    fd.id     = I;
    fd.isIO   = 1;
    fd.ispipe = 0;
    fd.size = 128;
    fd.ro = 0;
    pcb[new_id].fd_table[I] = fd;
  }

  pcb[new_id].fd_table[0].ro = 1;

  create_page_table(new_id);

  heap_push(process_queue, &(pcb[new_id]), 0);
  // heap_display(process_queue);

  return new_id;
}

void kill_process(ctx_t *ctx, int pid){
  dead_processes[pid] = 1;
  heap_remove(process_queue, &(pcb[pid]));
  // heap_display(process_queue);
  cleanup_process(pid);
  for(int I = 0; I < pcb[pid].pipe_amount;I++){
    memset(&(pcb[pid].pipes[I]), 0, 0x00100000);
  }
  pcb[pid].pipe_amount = 0;
  scheduler(ctx);
  process_amount--;
}

void switch_process(ctx_t* ctx, int id){
  memcpy( &pcb[current_process].ctx, ctx, sizeof(ctx_t));
  memcpy( ctx, &pcb[id].ctx, sizeof(ctx_t));
  switch_page_table(id);
  current_process = id;
  return;
}

int stdin_insert(char c){
  int head = stdin_head[focused_process];
  if (head == 127){
    if (c == '\n'){
      STDIN[focused_process][head] = c;
    }
    return 1;
  }
  STDIN[focused_process][head] = c;
  stdin_head[focused_process]++;
  return 0;
}