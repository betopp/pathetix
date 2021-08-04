//mmlibc/include/mmbits/define_sa_options.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _DEFINE_SA_OPTIONS_H
#define _DEFINE_SA_OPTIONS_H

//Like Linux.
#define SA_NOCLDSTOP 0x1
#define SA_ONSTACK 0x8000000
#define SA_RESETHAND 0x80000000
#define SA_RESTART 0x10000000
#define SA_SIGINFO 0x4
#define SA_NOCLDWAIT 0x2
#define SA_NODEFER 0x40000000
#define SS_ONSTACK 0x1
#define SS_DISABLE 0x2
#define MINSIGSTKSZ 0x800
#define SIGSTKSZ 0x2000

#endif //_DEFINE_SA_OPTIONS_H


