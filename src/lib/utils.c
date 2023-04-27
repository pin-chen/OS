#include <utils.h>

void wait_cycles(uint32_t times){
    while(times--) asm volatile("nop");
}

uint32_t hex2u32_8(char *buf){
    uint32_t num = 0;
    for(int i = 0; i < 8; i++){
        num <<= 4;
        num += (buf[i] >= 'A' ? buf[i] - 'A' + 10 : buf[i] - '0');
    }
    return num;
}