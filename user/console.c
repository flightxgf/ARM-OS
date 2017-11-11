#include "console.h"


/* The following functions are special-case versions of a) writing,
 * and b) reading a string from the UART (the latter case returning 
 * once a carriage return character has been read, or an overall
 * limit reached).
 */

// void puts( char* x, int n ) {
//   for( int i = 0; i < n; i++ ) {
//     PL011_putc( UART0, x[ i ], true );
//   }
// }

// void newgets( char* x, int n ) {
//   for( int i = 0; i < n; i++ ) {
//     x[ i ] = PL011_getc( UART0, true );
    
//     if( x[ i ] == '\x0D' ) {
//       x[ i ] = '\x00'; break;
//     }
//   }
// }

/* Since we lack a *real* loader (as a result of lacking a storage 
 * medium to store program images), the following approximates one:
 * given a program name, from the set of programs statically linked
 * into the kernel image, it returns a pointer to the entry point.
 */

extern void main_P3(); 
extern void main_P4(); 
extern void main_P5(); 
extern void main_prio_demo();
extern void main_cow_demo();
extern void main_philosophers();

void* load( char x[32]) {
  if     ( 0 == strcmp( x, "P3" ) ) {
    return &main_P3;
  }
  else if( 0 == strcmp( x, "P4" ) ) {
    return &main_P4;
  }
  else if( 0 == strcmp( x, "P5" ) ) {
    return &main_P5;
  }
  else if( 0 == strcmp( x, "Phil" ) ) {
    return &main_philosophers;
  }
  else if( 0 == strcmp( x, "DemoS" ) ) {
    return &main_prio_demo;
  }
  else if( 0 == strcmp( x, "DemoC" ) ) {
    return &main_cow_demo;
  }

  return NULL;
}

/* The behaviour of the console process can be summarised as an
 * (infinite) loop over three main steps, namely
 *
 * 1. write a command prompt then read a command,
 * 2. split the command into space-separated tokens using strtok,
 * 3. execute whatever steps the command dictates.
 */

void main_console() {
  char input[64];
  char command[32];
  char arg[32];

  while( 1 ) {
    printf("shell$ ");
    memset(input, 0, 64);
    memset(command, 0, 32);
    memset(arg, 0, 32);
    fflush(stdout);
    int I = 0;
    while (1){
      if(_read(STDIN_FILENO, &(input[I]), 1)){
        if (input[I] == '\n' || I == 64){
          input[I] = '\0';
          break;
        }
        I++;
      }
    }
    int divider = 0;
    for (int I = 0; I < 64; I++){
      if (input[I] == ' '){
        divider = I;
        break;
      }
    }
    memcpy(command, input, divider);
    memset(&(command[divider]), 0, 32 - divider);
    memcpy(arg, &(input[divider + 1]), I - divider);
    memset(&(arg[I - divider]), 0, 32 -  I - divider);
    printf("command is %s, arg is %s, divider is %d\n", command, arg, divider);

    if     ( 0 == strcmp( command, "fork" ) ) {
      uint32_t addr = load(arg);
      if (addr == 0){
        printf("Invalid program\n");
      }
      else{
        pid_t pid = fork();
        if( 0 == pid ) {
          exec( addr );
        }
      }
    } 
    else if( 0 == strcmp( command, "kill" ) ) {
      pid_t pid = atoi(arg);
      int   s   = atoi(arg);

      kill( pid, s );
    } 
    else {
      printf("unknown command\n");
    }
  }

  exit( EXIT_SUCCESS );
}
