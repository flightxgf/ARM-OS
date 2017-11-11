#include "prio_demo.h"

extern void main_low(); 
extern void main_med(); 
extern void main_high(); 

void main_prio_demo() {

  switch_sched();
  int pid = fork();
  if (pid == 0){
    exec(&main_low);
  }
  pid = fork();
  if (pid == 0){
    exec(&main_med);
  }
  pid = fork();
  if (pid == 0){
    exec(&main_high);
  }
  printf("=====Round robin=====\n");

  sleep(1);
  switch_sched();
  printf("=====Prio Heap=====\n");

  exit( EXIT_SUCCESS );
}
