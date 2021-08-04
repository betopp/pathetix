//syscalls.h
//System call implementation
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <stdint.h>

//Acts on a system-call action with the given parameters.
uint64_t syscalls_switch(uint64_t call, uint64_t p1, uint64_t p2, uint64_t p3, uint64_t p4, uint64_t p5);

#endif //SYSCALLS_H
