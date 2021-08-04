//mmlibc/include/mmbits/define_rlimit_options.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _DEFINE_RLIMIT_OPTIONS_H
#define _DEFINE_RLIMIT_OPTIONS_H

//Like Linux.
#define RLIMIT_CPU 0
#define RLIMIT_FSIZE 1
#define RLIMIT_DATA 2
#define RLIMIT_STACK 3
#define RLIMIT_CORE 4
#define RLIMIT_NOFILE 7
#define RLIMIT_AS 9

//Number of RLIMIT values
#define _RLIMIT_MAX 10

#endif //_DEFINE_RLIMIT_OPTIONS_H
