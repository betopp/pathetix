//mmlibc/include/mmbits/union_sigval.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _UNION_SIGVAL_H
#define _UNION_SIGVAL_H

union sigval
{
	int sival_int; //Integer signal value
	void *sival_ptr; //Pointer signal value
};

#endif //_UNION_SIGVAL_H

