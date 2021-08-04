//mmlibc/include/mmbits/typedef_div.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _TYPEDEF_DIV_H
#define _TYPEDEF_DIV_H

typedef struct _div_s
{
	int quot;
	int rem;
} div_t;

typedef struct _ldiv_s
{
	long quot;
	long rem;
} ldiv_t;

typedef struct _lldiv_s
{
	long long quot;
	long long rem;
} lldiv_t;

#endif //_TYPEDEF_DIV_H
