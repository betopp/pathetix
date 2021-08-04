//hal_atomic.h
//Atomic counters for kernel
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef HAL_ATOMIC_H
#define HAL_ATOMIC_H

//Type atomically counted on this architecture (todo - make arch-specific)
typedef uint32_t hal_atomic_t;

//Increments a counter and returns the new value
uint64_t hal_atomic_inc(hal_atomic_t *atom);

//Decrements a counter and returns the new value
uint64_t hal_atomic_dec(hal_atomic_t *atom);

#endif //HAL_ATOMIC_H
