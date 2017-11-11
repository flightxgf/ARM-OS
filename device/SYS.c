#include "SYS.h"

volatile SYSCONF_t* SYSCONF  = ( volatile SYSCONF_t* )( 0x90000000 );

volatile  uint32_t* SYSCTRL0 = ( volatile  uint32_t* )( 0x90001000 );
volatile  uint32_t* SYSCTRL1 = ( volatile  uint32_t* )( 0x9001A000 );
