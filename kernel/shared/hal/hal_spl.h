//hal_spl.h
//Spinlocks
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef HAL_SPL_H
#define HAL_SPL_H

#include <stdint.h>
#include <stdbool.h>

//Data stored for a spinlock.
typedef volatile uint64_t hal_spl_t;

//Locks the given spinlock.
void hal_spl_lock(hal_spl_t *spl);

//Tries to lock the given spinlock but gives up immediately if it's busy.
//Returns whether the lock was successfully acquired.
bool hal_spl_try(hal_spl_t *spl);

//Unlocks the given spinlock.
void hal_spl_unlock(hal_spl_t *spl);


#endif //HAL_SPL_H
