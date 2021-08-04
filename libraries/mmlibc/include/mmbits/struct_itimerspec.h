//mmlibc/include/mmbits/struct_itimerspec.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _STRUCT_ITIMERSPEC_H
#define _STRUCT_ITIMERSPEC_H

struct itimerspec
{
	struct timespec it_interval;  //Timer period. 
	struct timespec it_value;     //Timer expiration. 
};

#endif //_STRUCT_ITIMERSPEC_H
