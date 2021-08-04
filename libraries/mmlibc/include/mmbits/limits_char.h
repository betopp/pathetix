//mmlibc/include/mmbits/limits_char.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _LIMITS_CHAR_H
#define _LIMITS_CHAR_H

#define SCHAR_MAX (__SCHAR_MAX__)
#define SCHAR_MIN (-__SCHAR_MAX__-1)
#define UCHAR_MAX ((__SCHAR_MAX__*2)+1)

#ifdef __CHAR_UNSIGNED__
	#define CHAR_MAX UCHAR_MAX
	#define CHAR_MIN 0
#else
	#define CHAR_MAX SCHAR_MAX
	#define CHAR_MIN SCHAR_MIN
#endif

#endif //_LIMITS_CHAR_H
