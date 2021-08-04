//mmlibc/include/mmbits/limits_int_fastn.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _LIMITS_INT_FASTN_H
#define _LIMITS_INT_FASTN_H

#define INT_FAST8_MAX (__INT_FAST8_MAX__)
#define INT_FAST8_MIN (-__INT_FAST8_MAX__ - 1)

#define INT_FAST16_MAX (__INT_FAST16_MAX__)
#define INT_FAST16_MIN (-__INT_FAST16_MAX__ - 1)

#define INT_FAST32_MAX (__INT_FAST32_MAX__)
#define INT_FAST32_MIN (-__INT_FAST32_MAX__ - 1)

#define INT_FAST64_MAX (__INT_FAST64_MAX__)
#define INT_FAST64_MIN (-__INT_FAST64_MAX__ - 1)


#define UINT_FAST8_MAX (__UINT_FAST8_MAX__)
#define UINT_FAST16_MAX (__UINT_FAST16_MAX__)
#define UINT_FAST32_MAX (__UINT_FAST32_MAX__)
#define UINT_FAST64_MAX (__UINT_FAST64_MAX__)

#endif //_LIMITS_INT_FASTN_H
