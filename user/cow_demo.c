#include "cow_demo.h"


void main_cow_demo() {

  uint8_t *test = (uint8_t*)0;
  memset(test, 1, 3 * 0x00100000);

  int pid = fork();
  if (pid == 0){
    uint32_t sum = 0;
    for(int I = 0; I < 3 * 0x00100000;I++){
      sum += test[I];
    }
    printf("value: 0x%08x\n", sum);
    printf("writing duplicates pages\n");
    test[0x00210000] = 2;
    exit(0);
  }

  exit( EXIT_SUCCESS );
}
