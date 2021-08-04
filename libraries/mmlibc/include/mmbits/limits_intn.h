//mmlibc/include/mmbits/limits_intn.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _LIMITS_INTN_H
#define _LIMITS_INTN_H

#define INT8_MAX (__INT8_MAX__)
#define INT8_MIN (-__INT8_MAX__ - 1)

#define INT16_MAX (__INT16_MAX__)
#define INT16_MIN (-__INT16_MAX__ - 1)

#define INT32_MAX (__INT32_MAX__)
#define INT32_MIN (-__INT32_MAX__ - 1)

#define INT64_MAX (__INT64_MAX__)
#define INT64_MIN (-__INT64_MAX__ - 1)


#define UINT8_MAX (__UINT8_MAX__)
#define UINT16_MAX (__UINT16_MAX__)
#define UINT32_MAX (__UINT32_MAX__)
#define UINT64_MAX (__UINT64_MAX__)

#endif //_LIMITS_INTN_H
