//assert.c
//Assertion failure in libc
//Bryan E. Topp <betopp@betopp.com> 2021

#include <assert.h>

void _assertion_failed(const char *cond, const char *file, const char *func, int line)
{
	(void)cond;
	(void)file;
	(void)func;
	(void)line;
	while(1) { } //Todo - once we have a way to crash, crash
}
