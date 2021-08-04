//mmlibc/include/stdarg.h
//Variadic function definitions for MMK's libc
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _STDARG_H
#define _STDARG_H

//GCC and Clang define vararg handling as builtins.
#define va_start(ap,argn) __builtin_va_start(ap,argn)
#define va_copy(dest,src) __builtin_va_copy(dest,src)
#define va_arg(ap,type) __builtin_va_arg(ap,type)
#define va_end(ap) __builtin_va_end(ap)

#include <mmbits/typedef_va_list.h>

#endif //_STDARG_H
