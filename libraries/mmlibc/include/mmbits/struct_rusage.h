//mmlibc/include/mmbits/struct_rusage.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _STRUCT_RUSAGE_H
#define _STRUCT_RUSAGE_H

#include <mmbits/struct_timeval.h>

struct rusage
{
	struct timeval ru_utime;
	struct timeval ru_stime;
};

#endif //_STRUCT_RUSAGE_H
