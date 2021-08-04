//mmlibc/include/mmbits/typedef_idtype.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _TYPEDEF_IDTYPE_H
#define _TYPEDEF_IDTYPE_H

//Like Linux
typedef enum _idtype_e
{
	P_ALL = 0x0,
	P_PID = 0x1,
	P_PGID = 0x2,
	P_TID = 0xB,
} idtype_t;

#endif //_TYPEDEF_IDTYPE_H

