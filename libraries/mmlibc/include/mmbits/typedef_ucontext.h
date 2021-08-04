//mmlibc/include/mmbits/typedef_ucontext.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _TYPEDEF_UCONTEXT_H
#define _TYPEDEF_UCONTEXT_H

#include <mmbits/typedef_mcontext.h>
#include <mmbits/typedef_stack.h>
#include <mmbits/typedef_sigset.h>

typedef struct _ucontext_s
{
	struct _ucontext_s *uc_link; //Pointer to context that is resumed when this one finishes
	sigset_t uc_sigmask; //Set of signals blocked when this context is active
	stack_t uc_stack; //Stack used by this context
	mcontext_t uc_mcontext; //Machine-specific saved context
} ucontext_t;

#endif //_TYPEDEF_UCONTEXT_H


