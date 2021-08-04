//mmlibc/include/mmbits/struct_sigaction.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _STRUCT_SIGACTION_H
#define _STRUCT_SIGACTION_H

#include <mmbits/typedef_siginfo.h>

struct sigaction
{
	void (*sa_handler)(int); //Pointer to a signal-catching function or SIG_IGN or DIG_DFL.
	sigset_t sa_mask; //Set of signals blocked during execution of the signal handling function.
	int sa_flags; //Special flags
	void (*sa_sigaction)(int, siginfo_t *, void *); //Pointer to a signal-catching function.
};

#endif //_STRUCT_SIGACTION_H


