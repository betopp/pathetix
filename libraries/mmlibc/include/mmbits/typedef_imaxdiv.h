//mmlibc/include/mmbits/typedef_imaxdiv.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _TYPEDEF_IMAXDIV_H
#define _TYPEDEF_IMAXDIV_H

#include <mmbits/typedef_intmax.h>

typedef struct _imaxdiv_s
{
	intmax_t quot;
	intmax_t rem;
} imaxdiv_t;

#endif //_TYPEDEF_IMAXDIV_H
