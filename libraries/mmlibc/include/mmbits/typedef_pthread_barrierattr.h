//mmlibc/include/mmbits/typedef_pthread_barrierattr.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _TYPEDEF_PTHREAD_BARRIERATTR_H
#define _TYPEDEF_PTHREAD_BARRIERATTR_H

typedef struct {
	//The base POSIX spec doesn't actually require anything here, but requires the structure...?
	int dummy;
} pthread_barrierattr_t;

#endif //_TYPEDEF_PTHREAD_BARRIERATTR_H
