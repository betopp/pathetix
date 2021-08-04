//mmlibc/include/mmbits/typedef_pthread_mutexattr.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _TYPEDEF_PTHREAD_MUTEXATTR_H
#define _TYPEDEF_PTHREAD_MUTEXATTR_H

typedef struct
{
	int robustness;
	int type;	
} pthread_mutexattr_t;

#endif //_TYPEDEF_PTHREAD_MUTEXATTR_H
