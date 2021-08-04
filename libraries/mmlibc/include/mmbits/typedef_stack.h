//mmlibc/include/mmbits/typedef_stack.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _TYPEDEF_STACK_H
#define _TYPEDEF_STACK_H

typedef struct _stack_s
{
	void *ss_sp; //Stack base or pointer
	size_t ss_size; //Stack size
	int ss_flags; //Flags
} stack_t;

#endif //_TYPEDEF_STACK_H


