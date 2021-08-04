//mmlibc/include/mmbits/struct_timeval.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _STRUCT_TIMEVAL_H
#define _STRUCT_TIMEVAL_H

#include <mmbits/typedef_time.h>
#include <mmbits/typedef_suseconds.h>

struct timeval
{
	time_t tv_sec; //Seconds
	suseconds_t tv_usec; //Microseconds
};

#endif //_STRUCT_TIMEVAL_H
