//mmlibc/include/mmbits/typedef_pthread_attr.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _TYPEDEF_PTHREAD_ATTR_H
#define _TYPEDEF_PTHREAD_ATTR_H

#include <mmbits/typedef_size.h>
#include <mmbits/struct_sched_param.h>

typedef struct
{
	int detachstate;
	size_t guardsize;
	struct sched_param schedparam;	
} pthread_attr_t;

#endif //_TYPEDEF_PTHREAD_ATTR_H
