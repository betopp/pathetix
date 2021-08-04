//mmlibc/include/mmbits/constmac_intn.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _CONSTMAC_INTN_H
#define _CONSTMAC_INTN_H

#define INT8_C(x) ((int_least8_t)(x))
#define INT16_C(x) ((int_least16_t)(x))
#define INT32_C(x) ((int_least32_t)(x))
#define INT64_C(x) ((int_least64_t)(x))

#define UINT8_C(x) ((uint_least8_t)(x))
#define UINT16_C(x) ((uint_least16_t)(x))
#define UINT32_C(x) ((uint_least32_t)(x))
#define UINT64_C(x) ((uint_least64_t)(x))

#endif //_CONSTMAC_INTN_H
