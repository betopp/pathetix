//mmlibc/include/mmbits/struct_rlimit.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _STRUCT_RLIMIT_H
#define _STRUCT_RLIMIT_H

#include <mmbits/typedef_rlim.h>

struct rlimit
{
	rlim_t rlim_cur; //The current (soft) limit
	rlim_t rlim_max; //The hard limit
};

#endif //_STRUCT_RLIMIT_H
