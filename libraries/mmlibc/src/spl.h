//spl.h
//Spinlocks for MuKe's libc
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef _SPL_H
#define _SPL_H

//Data for spinlock
typedef volatile uint64_t _spl_t;

//Locks the given spinlock.
void _spl_lock(_spl_t *spl);

//Unlocks the given spinlock.
void _spl_unlock(_spl_t *spl);

#endif //_SPL_H
