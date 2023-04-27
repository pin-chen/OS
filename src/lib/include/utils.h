#ifndef __UTILS__
#define __UTILS__
#include <stddef.h>
#include <stdint.h>
#define memory_read(addr) *(uint32_t *)(addr)
#define memory_write(addr,val) *(uint32_t *)(addr) = (uint32_t)(val)
void wait_cycles(uint32_t times);
uint32_t hex2u32_8(char *buf);
#endif