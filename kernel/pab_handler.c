#include "pab_handler.h"

void pab_handler(uint32_t addr, ctx_t *ctx) {
  printf("HANDLING PREFETCH ABORT FOR 0x%08x FROM PROCESS %d\n", addr, current_process);
  uint32_t page = addr / 0x00100000;
  // if(swapped_pages[current_process][page] != 0){
  //   //load from swap
  // }
  // else{
    printf("PROGRAM %d ACCESSED MEMORY WITHOUT PERMISSION\n", current_process);
    printf("PROGRAM %d WILL BE EXECUTED AS AN EXAMPLE TO OTHERS\n", current_process);
    kill_process(ctx, current_process);
  // }
  sleep(1);
  return;
}