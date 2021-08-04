//mmlibc/include/mmbits/typedef_time.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _TYPEDEF_TIME_H
#define _TYPEDEF_TIME_H

//time_t is always 64-bit on DUCK.
typedef __INT64_TYPE__ time_t;

#endif //_TYPEDEF_TIME_H
