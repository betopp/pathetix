//mmlibc/include/mmbits/define_fdset_funcs.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _DEFINE_FDSET_FUNCS_H
#define _DEFINE_FDSET_FUNCS_H

#include <mmbits/typedef_fdset.h>

void _fd_clr(int fd, fd_set *fdset);
int _fd_isset(int fd, fd_set *fdset);
void _fd_set(int fd, fd_set *fdset);
void _fd_zero(fd_set *fdset);

#define FD_CLR(fd, fdset) _fd_clr(fd, fdset)
#define FD_ISSET(fd, fdset) _fd_isset(fd, fdset)
#define FD_SET(fd, fdset) _fd_set(fd, fdset)
#define FD_ZERO(fdset) _fd_zero(fdset)

#endif //_DEFINE_FDSET_FUNCS_H
