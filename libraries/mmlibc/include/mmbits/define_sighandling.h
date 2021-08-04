//mmlibc/include/mmbits/define_sighandling.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _DEFINE_SIGHANDLING_H
#define _DEFINE_SIGHANDLING_H

//Like Linux.
#define SIG_DFL ((void (*)(int))(0))
#define SIG_ERR ((void (*)(int))(-1))
#define SIG_IGN ((void (*)(int))(1))

#endif //_DEFINE_SIGHANDLING_H
