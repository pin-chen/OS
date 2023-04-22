#include <utils.h>

void wait_cycles(uint32_t times){
    while(times--) asm volatile("nop");
}

