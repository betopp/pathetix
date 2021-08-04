//mmlibc/include/mmbits/typedef_pthread_barrier.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _TYPEDEF_PTHREAD_BARRIER_H
#define _TYPEDEF_PTHREAD_BARRIER_H

#include <mmbits/typedef_pthread_barrierattr.h>

typedef struct
{
	int count;
	pthread_barrierattr_t attr;
} pthread_barrier_t;

#endif //_TYPEDEF_PTHREAD_BARRIER_H
