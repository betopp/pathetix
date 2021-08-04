//mmlibc/include/mmbits/define_fopen_max.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _DEFINE_FOPEN_MAX_H
#define _DEFINE_FOPEN_MAX_H

//POSIX specifies constants that "have the same value as FOPEN_MAX" in headers that don't themselves define FOPEN_MAX.
//So define it like this, then define it in those headers with appropriate public names.
#define _FOPEN_MAX 256

#endif //_DEFINE_FOPEN_MAX_H
