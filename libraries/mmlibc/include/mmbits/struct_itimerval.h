//mmlibc/include/mmbits/struct_itimerval.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _STRUCT_ITIMERVAL_H
#define _STRUCT_ITIMERVAL_H

#include <mmbits/struct_timeval.h>

struct itimerval
{
	struct timeval it_interval; //Timer interval
	struct timeval it_value; //Current value
};

#endif //_STRUCT_ITIMERVAL_H
