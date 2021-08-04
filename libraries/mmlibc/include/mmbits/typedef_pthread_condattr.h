//mmlibc/include/mmbits/typedef_pthread_condattr.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _TYPEDEF_PTHREAD_CONDATTR_H
#define _TYPEDEF_PTHREAD_CONDATTR_H

#include <mmbits/typedef_clockid.h>

typedef struct
{
	clockid_t clock_id;	
} pthread_condattr_t;

#endif //_TYPEDEF_PTHREAD_CONDATTR_H
