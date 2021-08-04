//mmlibc/include/mmbits/typedef_fdset.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _TYPEDEF_FDSET_H
#define _TYPEDEF_FDSET_H

#include <mmbits/define_fdsetsize.h>

typedef struct _fd_set_s
{
	int fds[FD_SETSIZE];
} fd_set;

#endif //_TYPEDEF_FDSET_H
