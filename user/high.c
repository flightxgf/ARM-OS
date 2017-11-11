#include "high.h"

// int is_prime( uint32_t x ) {
//   if ( !( x & 1 ) || ( x < 2 ) ) {
//     return ( x == 2 );
//   }

//   for( uint32_t d = 3; ( d * d ) <= x ; d += 2 ) {
//     if( !( x % d ) ) {
//       return 0;
//     }
//   }

//   return 1;
// }

void main_high() {
  set_priority(PRIORITY_HIGH);
  for( int i = 0; i < 25; i++ ) {
    printf("High priority executing %d\n", i);
    uint32_t lo = 1 <<  8;
    uint32_t hi = 1 << 16;

    for( uint32_t x = lo; x < hi; x++ ) {
      int r = is_prime( x ); 
    }
  }

  exit( EXIT_SUCCESS );
}
