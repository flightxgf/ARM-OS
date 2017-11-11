#include "libc.h"

// int  atoi( char* x        ) {
//   char* p = x; bool s = false; int r = 0;

//   if     ( *p == '-' ) {
//     s =  true; p++;
//   }
//   else if( *p == '+' ) {
//     s = false; p++;
//   }

//   for( int i = 0; *p != '\x00'; i++, p++ ) {
//     r = s ? ( r * 10 ) - ( *p - '0' ) :
//             ( r * 10 ) + ( *p - '0' ) ;
//   }

//   return r;
// }

void itoa( char* r, int x ) {
  char* p = r; int t, n;

  if( x < 0 ) {
    p++; t = -x; n = 1;
  }
  else {
         t = +x; n = 1;
  }

  while( t >= n ) {
    p++; n *= 10;
  }

  *p-- = '\x00';

  do {
    *p-- = '0' + ( t % 10 ); t /= 10;
  } while( t );

  if( x < 0 ) {
    *p-- = '-';
  }

  return;
}

void yield() {
  asm volatile( "stmdb sp!, {lr}\n"
                "mrs   r0,  spsr\n"
                "stmdb sp!, {r0}\n"
                "svc %0     \n" // make system call SYS_YIELD
                "ldmia sp!, {r1}\n"
                "ldmia sp!, {lr}\n"
              :
              : "I" (SYS_YIELD)
              : "r0", "r1");

  return;
}

int _write( int fd, const void* x, size_t n ) {
  int r;

  asm volatile( "stmdb sp!, {lr}\n"
                "mrs   r0,  cpsr\n"
                "stmdb sp!, {r0}\n"
                "mov r0, %2 \n" // assign r0 = fd
                "mov r1, %3 \n" // assign r1 =  x
                "mov r2, %4 \n" // assign r2 =  n
                "svc %1     \n" // make system call SYS_WRITE
                "mov %0, r0 \n" // assign r  = r0
                "ldmia sp!, {r1}\n"
                "ldmia sp!, {lr}\n"
              : "=r" (r) 
              : "I" (SYS_WRITE), "r" (fd), "r" (x), "r" (n)
              : "r0", "r1", "r2" );

  return r;
}

int  _read( int fd,       void* x, size_t n ) {
  volatile int r;

  asm volatile( "stmdb sp!, {lr}\n"
                "mrs   r0,  cpsr\n"
                "stmdb sp!, {r0}\n"
                "mov r0, %2 \n" // assign r0 = fd
                "mov r1, %3 \n" // assign r1 =  x
                "mov r2, %4 \n" // assign r2 =  n
                "svc %1     \n" // make system call SYS_READ
                "mov %0, r0 \n" // assign r  = r0
                "ldmia sp!, {r1}\n"
                "ldmia sp!, {lr}\n"
              : "=r" (r) 
              : "I" (SYS_READ),  "r" (fd), "r" (x), "r" (n) 
              : "r0", "r1", "r2" );

  return r;
}

int fork() {
  int r;

  asm volatile( "stmdb sp!, {lr}\n"
                "mrs   r0,  cpsr\n"
                "stmdb sp!, {r0}\n"
                "svc %1     \n" // make system call SYS_FORK
                "mov %0, r0 \n" // assign r  = r0 
                "ldmia sp!, {r1}\n"
                "ldmia sp!, {lr}\n"
              : "=r" (r) 
              : "I" (SYS_FORK)
              : "r0", "r1" );

  return r;
}

void exit( int x ) {
  asm volatile( "mov r0, %1 \n" // assign r0 =  x
                "svc %0     \n" // make system call SYS_EXIT
              :
              : "I" (SYS_EXIT), "r" (x)
              : "r0", "r1" );

  return;
}

void exec( const void* x ) {
  asm volatile( "stmdb sp!, {lr}\n"
                "mrs   r0,  cpsr\n"
                "stmdb sp!, {r0}\n"
                "mov r0, %1 \n" // assign r0 = x
                "svc %0     \n" // make system call SYS_EXEC
                "ldmia sp!, {r1}\n"
                "ldmia sp!, {lr}\n"
              :
              : "I" (SYS_EXEC), "r" (x)
              : "r0", "r1" );

  return;
}

int kill( int pid, int x ) {
  int r;

  asm volatile( 
                "mov r0, %2 \n" // assign r0 =  pid
                "mov r1, %3 \n" // assign r1 =    x
                "svc %1     \n" // make system call SYS_KILL
                "mov %0, r0 \n" // assign r0 =    r
              : "=r" (r) 
              : "I" (SYS_KILL), "r" (pid), "r" (x)
              : "r0", "r1" );

  return r;
}

int set_priority(int prio){
  int r;
  asm volatile( "stmdb sp!, {lr}\n"
                "mrs   r0,  cpsr\n"
                "stmdb sp!, {r0}\n"
                "mov r0, %2 \n" // assign r0 =  pid
                "svc %1     \n" // make system call SYS_KILL
                "mov %0, r0 \n" // assign r0 =    r
                "ldmia sp!, {r1}\n"
                "ldmia sp!, {lr}\n"
              : "=r" (r) 
              : "I" (SYS_PRIO), "r" (prio)
              : "r0", "r1" );

  return r;
}

int get_priority(){
  int r;
  asm volatile( "stmdb sp!, {lr}\n"
                "mrs   r0,  cpsr\n"
                "stmdb sp!, {r0}\n"
                "svc %1     \n" // make system call SYS_KILL
                "mov %0, r0 \n" // assign r0 =    r
                "ldmia sp!, {r1}\n"
                "ldmia sp!, {lr}\n"
              : "=r" (r) 
              : "I" (SYS_PRIO), "r" (-1)
              : "r0", "r1", "r2" );

  return r;
}

caddr_t _sbrk(int increment) {
  int r;

  asm volatile( "stmdb sp!, {lr}\n"
                "mrs   r0,  cpsr\n"
                "stmdb sp!, {r0}\n"
                "mov r0, %2 \n" // assign r0 =  pid
                "svc %1     \n" // make system call SYS_KILL
                "mov %0, r0 \n" // assign r0 =    r
                "ldmia sp!, {r1}\n"
                "ldmia sp!, {lr}\n"
              : "=r" (r) 
              : "I" (SYS_SBRK), "r" (increment)
              : "r0", "r1" );

  return r;
}

int pipe_create(int pid) {
  asm volatile( "stmdb sp!, {lr}\n"
                "mrs   r0,  cpsr\n"
                "stmdb sp!, {r0}\n"
                "mov r0, %1 \n" // assign r0 = x
                "svc %0     \n" // make system call SYS_EXEC
                "ldmia sp!, {r1}\n"
                "ldmia sp!, {lr}\n"
              :
              : "I" (SYS_PCRT), "r" (pid)
              : "r0", "r1" );

  return;
}

int pipe_attach(int pid) {
  asm volatile( "stmdb sp!, {lr}\n"
                "mrs   r0,  cpsr\n"
                "stmdb sp!, {r0}\n"
                "mov r0, %1 \n" // assign r0 = x
                "svc %0     \n" // make system call SYS_EXEC
                "ldmia sp!, {r1}\n"
                "ldmia sp!, {lr}\n"
              :
              : "I" (SYS_PATT), "r" (pid)
              : "r0", "r1" );

  return;
}

int pid() {
  int r;

  asm volatile( "stmdb sp!, {lr}\n"
                "mrs   r0,  cpsr\n"
                "stmdb sp!, {r0}\n"
                "svc %1     \n" // make system call SYS_FORK
                "mov %0, r0 \n" // assign r  = r0 
                "ldmia sp!, {r1}\n"
                "ldmia sp!, {lr}\n"
              : "=r" (r) 
              : "I" (SYS_PID)
              : "r0", "r1" );

  return r;
}

int switch_sched() {
  int r;

  asm volatile( "stmdb sp!, {lr}\n"
                "mrs   r0,  cpsr\n"
                "stmdb sp!, {r0}\n"
                "svc %1     \n" // make system call SYS_FORK
                "mov %0, r0 \n" // assign r  = r0 
                "ldmia sp!, {r1}\n"
                "ldmia sp!, {lr}\n"
              : "=r" (r) 
              : "I" (SYS_SWS)
              : "r0", "r1" );

  return r;
}