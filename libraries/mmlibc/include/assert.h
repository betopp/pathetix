//mmlibc/include/assert.h
//Assertion declarations for MMK's libc.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _ASSERT_H
#define _ASSERT_H

#ifdef NDEBUG
	#define assert(ignore) ((void)0)
#else
	extern void _assertion_failed(const char *cond, const char *file, const char *func, int line);
	#define assert(x)   do { if(!(x)) { _assertion_failed(#x, __FILE__, __func__, __LINE__); } } while(0)
#endif

#endif // _ASSERT_H
