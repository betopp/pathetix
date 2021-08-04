//mmlibc/include/mmbits/struct_flock.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _STRUCT_FLOCK_H
#define _STRUCT_FLOCK_H

#include <mmbits/typedef_off.h>
#include <mmbits/typedef_pid.h>

struct flock
{
	short l_type;
	short l_whence;
	off_t l_start;
	off_t l_len;
	pid_t l_pid;
};

#endif //_STRUCT_FLOCK_H
