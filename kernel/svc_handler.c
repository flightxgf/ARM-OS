#include "svc_handler.h"


int __attribute__((optimize("O0"))) svc_handler(int arg0, int arg1, int arg2, ctx_t* ctx, int code, int processor_mode) {
  if (code == SYS_WRITE){
    if (arg0 == STDIN_FILENO){
      printf("CAN'T WRITE TO STDIN\n");
    }
    else if (arg0 == STDOUT_FILENO){
      for(int I = 0; I < arg2; I++){
        PL011_putc(UART0, ((char*)arg1)[I], true);
      }
    }
    else if (arg0 == STDERR_FILENO){
      for(int I = 0; I < arg2; I++){
        PL011_putc(UART0, ((char*)arg1)[I], true);
      }
    }

    else{
      file_descriptor fd = pcb[current_process].fd_table[arg0];
      if (fd.ispipe){
        // printf("Wrote %d bytes to 0x%08x\n", arg2, current_page_table[(uint32_t)(fd.pipe) >> 20]);
        memcpy((char *)(fd.pipe->data + fd.pipe->head), (char*)arg1,  arg2); // don't write too much okay
        pcb[current_process].fd_table[arg0].pipe->head += arg2;
      }
    }

    ctx->gpr[0] = arg2;
    return 0;
  }

  if (code == SYS_READ){
    if (arg0 == STDIN_FILENO){
      int head = stdin_head[current_process];
      if (!head){
        ctx->gpr[0] = 0;
        return 0;
      }
      int read = 0;
      for (int I = 0; I < arg2 && I < 127 && I < head; I++){
        char c = STDIN[current_process][I];
        ((char*)arg1)[I] = c;
        read++;
      }
      memcpy(STDIN[current_process], &(STDIN[current_process][arg2]), head - arg2);
      stdin_head[current_process] = head - arg2; 
      ctx->gpr[0] = read;
      return 0;
    }
    else if (arg0 == STDOUT_FILENO){
      printf("CAN'T READ FROM STDOUT\n");
    }
    else if (arg0 == STDERR_FILENO){
      printf("CAN'T READ FROM STDERR\n");
    }
    else{
      file_descriptor fd = pcb[current_process].fd_table[arg0];
      if (fd.ispipe)
      if (arg2 <= fd.pipe->head){
        // printf("Read %d bytes from 0x%08x\n", arg2, current_page_table[(uint32_t)(fd.pipe) >> 20]);
        memcpy((char*)arg1, (char *)(fd.pipe->data), arg2);
        memmove((char*)(fd.mem_addr), (char*)(fd.pipe->data + arg2 + 1), fd.pipe->head - arg2);
        ctx->gpr[0] = arg2;
        fd.pipe->head -= arg2;
        return arg2;
      }
      else{
        ctx->gpr[0] = 0;
        return 0;
      }
    }

    return 0;
  }

  if (code == SYS_SBRK){
    // working_printf("Call to sbrk from mode: %d\n", processor_mode);
    if (processor_mode == 0x10){
      uint32_t old = pcb[current_process].heap_current;
      pcb[current_process].heap_current += arg0;
      ctx->gpr[0] = old;
      return old;
    }
    else{
      uint32_t old = kernel_heap;
      kernel_heap += arg0;
      ctx->gpr[0] = old;
      return old;
    }
  }

  // printf("PROCESS %d EXECUTING SYSCALL %o\n", current_process, code);


  if (code == SYS_YIELD){
    // printf("YIELD\n");
    scheduler(ctx);
    return 0;
  }


  if (code == SYS_FORK){
    //allocate a new PCB
    int new_process = add_process(0);

    //copy the whole pcb of the old process
    memcpy( &pcb[new_process].ctx, ctx, sizeof(ctx_t));
    
    // create_page_table(new_process);
    fork_copy_ro(current_process, new_process);

    //sort out where the new stack should go
    // uint32_t old_process_stack_origin = pcb[current_process].stack_top;
    // uint32_t stack_size = old_process_stack_origin - ctx->sp;
    // uint32_t old_process_stack_end = old_process_stack_origin - stack_size;
    // uint32_t new_process_stack_origin = old_process_stack_origin; //TODO FIXME URGENT
    // uint32_t new_process_stack_end = new_process_stack_origin - stack_size;
    // //and update the pcb with that new value
    // pcb[new_process].ctx.sp = new_process_stack_end;

    //do a full copy of the old stack to the new location
    // memcpy((char*)(new_process_stack_end), (char*)(old_process_stack_end), stack_size);
    
    //set the return value for the parent process because it doesn't get to return properly
    ctx->gpr[0] = new_process;

    //set the new process as active
    switch_process(ctx, new_process);
    
    //return value for child should be 0
    ctx->gpr[0] = 0;
    return 0;
  }

  if (code == SYS_EXIT){
    printf("TERMINATING PROCESS %d\n", current_process);
    kill_process(ctx, current_process);

    // printf("EXIT\n");
    return 0;
  }

  if (code == SYS_EXEC){
    ctx->pc = arg0;
    // printf("EXEC\n");
    printf("CHANGING EXECUTION TO 0x%08x\n", arg0);
    return 0;
  }

  if (code == SYS_KILL){
    printf("KILLING PROCESS %d WITH CODE %d\n", arg0, arg1);
    kill_process(ctx, arg0);

    // printf("KILL\n");
    return 0;
  }

  if (code == SYS_PRIO){
    printf("PRIO\n");
    printf("SETTING PRIORITY OF PROCESS %d TO %d\n", current_process, arg0);
    if (arg0 > -1 && arg0 < 6){
      pcb[current_process].priority = arg0;
    }
    ctx->gpr[0] = pcb[current_process].priority;
    return 0;
  }

  if (code == SYS_PCRT){
    printf("Creating pipe between %d and %d\n", current_process, arg0);
    uint32_t addr = allocate_new_shared_page(current_process, arg0); 
    switch_process(ctx, current_process);
    printf("Pipe memory is at 0x%08x\n", current_page_table[0xa00 + pcb[current_process].pipe_amount]);
    pcb[current_process].pipes[pcb[current_process].pipe_amount].host = current_process;
    pcb[current_process].pipes[pcb[current_process].pipe_amount].client = arg0;
    pcb[current_process].fd_amount++;
    pcb[current_process].fd_table = realloc(pcb[current_process].fd_table, pcb[current_process].fd_amount * sizeof(file_descriptor));
    file_descriptor fd;
    fd.head = 0;
    fd.id = pcb[current_process].fd_amount - 1;
    fd.ispipe = 1;
    fd.pipe = (pipe_t*)(0xa0000000 +  (0x00100000 * pcb[current_process].pipe_amount));
    fd.isIO = 0;
    fd.ro = 0;
    uint32_t size = 1048571;
    memcpy(&(pcb[current_process].fd_table[pcb[current_process].fd_amount - 1]), &fd, sizeof(file_descriptor));
    pcb[current_process].pipe_amount++;
    pcb[arg0].pipe_amount++;
    ctx->gpr[0] = pcb[current_process].fd_amount - 1;
    return 0;
  }

  if (code == SYS_PATT){
    for(int I = 0; I < pcb[current_process].pipe_amount; I++){
      if (pcb[current_process].pipes[I].host == arg0){
        pcb[current_process].fd_amount++;
        pcb[current_process].fd_table = realloc(pcb[current_process].fd_table, pcb[current_process].fd_amount * sizeof(file_descriptor));
        file_descriptor fd;
        fd.head = 0;
        fd.id = pcb[current_process].fd_amount - 1;
        fd.ispipe = 1;
        fd.pipe = &(pcb[current_process].pipes[I]);
        fd.isIO = 0;
        fd.ro = 1;
        uint32_t size = 1048571;
        memcpy(&(pcb[current_process].fd_table[pcb[current_process].fd_amount - 1]), &fd, sizeof(file_descriptor));
        ctx->gpr[0] = pcb[current_process].fd_amount - 1;
        return 0;
      }
    }
    ctx->gpr[0] = -1;
    return -1;
  }

  if (code == SYS_PID){
    ctx->gpr[0] = current_process;
    return current_process;
  }

  if (code == SYS_SWS){
    scheduler_mode = (scheduler_mode + 1) % 2;  
    return;
  }

  printf("UNHANDLED SYSCALL %08x\n", code);

   return 0;
}