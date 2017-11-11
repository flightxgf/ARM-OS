#include "time.h"

void sleep(int x){
    for (int I = 0; I < x; I++){
        for(int J = 0; J < 0x20000000; J++){
            asm volatile ("nop");
        }
    }
}