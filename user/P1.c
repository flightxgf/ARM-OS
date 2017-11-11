#include "P1.h"

void main_P1() {
  while( 1 ) {
    printf("Program 1 executing\n");
    asm volatile ("svc 0");
  }

  exit( EXIT_SUCCESS );
}
