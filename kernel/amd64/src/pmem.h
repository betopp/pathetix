//pmem.h
//Physical memory access
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef PMEM_H
#define PMEM_H

#include <stdint.h>

//Reads from the given physical address.
uint64_t pmem_read(uint64_t paddr);

//Writes to the given physical address.
void pmem_write(uint64_t paddr, uint64_t data);

//Zeroes the given physical frame.
void pmem_clrframe(uint64_t paddr);

#endif //PMEM_H
