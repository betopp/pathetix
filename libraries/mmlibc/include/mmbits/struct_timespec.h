//mmlibc/include/mmbits/struct_timespec.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _STRUCT_TIMESPEC_H
#define _STRUCT_TIMESPEC_H

#include <mmbits/typedef_time.h>

struct timespec
{
	time_t tv_sec;
	long tv_nsec;
};

#endif //_STRUCT_TIMESPEC_H
