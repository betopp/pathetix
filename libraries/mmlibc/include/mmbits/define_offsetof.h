//mmlibc/include/mmbits/define_offsetof.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _DEFINE_OFFSETOF_H
#define _DEFINE_OFFSETOF_H

#define offsetof(type, memberdesignator) ((size_t)(&(((type*)0)->memberdesignator)))

#endif //_DEFINE_OFFSETOF_H
