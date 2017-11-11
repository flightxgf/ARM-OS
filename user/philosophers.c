#include "philosophers.h"

uint32_t philosophers[16];
uint32_t waiter_pid;

void main_philosophers(){
  uint32_t philo_number;
  for(int I = 0; I < 16; I++){
    uint32_t ret;
    ret = fork();
    if (ret != 0){
      philo_number = I;
      philosophers[philo_number] = pid();
      waiter_pid = pid() + 16 - I;
      printf("Philosopher %d is process %d\n", I, pid());
      dine(philo_number);
    }
  }

  printf("Waiter is ready\n", pid());
  set_priority(PRIORITY_HIGH);
  int write_pipe_fd[16];
  int read_pipe_fd[16];
  for(int I = 0; I < 16; I++){
    read_pipe_fd[I] = -1;
    while(read_pipe_fd[I] == -1){
      read_pipe_fd[I] = pipe_attach(philosophers[I]);
    }
  }
  for(int I = 0; I < 16; I++){
    write_pipe_fd[I] = pipe_create(philosophers[I]);
  }

  for(int I = 0; I < 16; I++){
    char ack[4];
    while(!_read(read_pipe_fd[I], &ack, 4)){}
  }

  printf("Waiter ready\n");
  for(int I = 0; I < 10; I++){
    printf("Starting course %d\n", I);
    for(int J = 0; J < 16; J++){
      if ((J % 2 + I % 2) % 2){
        _write(write_pipe_fd[J], "eat", 4);
      }
      else{
        _write(write_pipe_fd[J], "nop", 4);
      }
    }
    for(int J = 0; J < 16; J++){
      char ack[4];
      while(!_read(read_pipe_fd[J], &ack, 4)){}
    }
  }

  for(int I = 0; I < 16; I++){
    _write(write_pipe_fd[I], "stp", 4);
  }

  exit(0);
}


void dine(uint32_t philo_number){
  printf("Philosopher %d seated and ready\n", philo_number);
  int read_pipe_fd = -1;
  int write_pipe_fd = pipe_create(waiter_pid);

  printf("Philosopher %d created pipe\n", philo_number);

  while(read_pipe_fd == -1){
    read_pipe_fd = pipe_attach(waiter_pid);
  }
  
  uint32_t done = 0;
  _write(write_pipe_fd, "ack", 4);

  printf("Philosopher %d ready\n", philo_number);

  while (!done){
    char req[4];
    while(!_read(read_pipe_fd, &req, 4)){}
    if (strcmp(req, "eat") == 0){
      printf("Philosopher %d eating\n", philo_number);
      _write(write_pipe_fd, "ack", 4);
    }
    else if (strcmp(req, "nop") == 0){
      printf("Philosopher %d starving\n", philo_number);
      _write(write_pipe_fd, "ack", 4);
    }
    else if (strcmp(req, "stp") == 0){
      done = 1; 
    }
    else{
      printf("Unrecognised command %s for philosopher %d\n",req, philo_number);
    }
  }
  exit(0);
}