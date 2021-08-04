//mmlibc/include/mmbits/typedef_pthread_rwlockattr.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _TYPEDEF_PTHREAD_RWLOCKATTR_H
#define _TYPEDEF_PTHREAD_RWLOCKATTR_H

typedef struct
{
	//The base POSIX spec doesn't actually require anything here, but requires the structure...?
	int dummy;
	
} pthread_rwlockattr_t;

#endif //_TYPEDEF_PTHREAD_RWLOCKATTR_H
