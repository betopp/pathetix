//mmlibc/include/mmbits/define_errno.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _DEFINE_ERRNO_H
#define _DEFINE_ERRNO_H

//errno needs to be thread-local.
#include <tls.h>
#define errno (_tls()->errno)

#endif //_DEFINE_ERRNO_H


