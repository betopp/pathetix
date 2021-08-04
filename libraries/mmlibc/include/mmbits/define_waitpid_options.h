//mmlibc/include/mmbits/define_waitpid_options.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _DEFINE_WAITPID_OPTIONS_H
#define _DEFINE_WAITPID_OPTIONS_H

//Like Linux.
#define WCONTINUED 0x8 //Can also be used with waitid, see define_waitid_options.h
#define WNOHANG 0x1    //Can also be used with waitid, see define_waitid_options.h
#define WUNTRACED 0x2

#endif //_DEFINE_WAITPID_OPTIONS_H

