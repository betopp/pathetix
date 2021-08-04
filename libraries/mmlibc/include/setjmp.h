//mmlibc/include/setjmp.h
//Long-jump declarations for MMK's libc
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _SETJMP_H
#define _SETJMP_H

#include <mmbits/typedef_jmpbuf.h>
#include <mmbits/typedef_sigjmpbuf.h>

void longjmp(jmp_buf env, int val);
void siglongjmp(sigjmp_buf env, int val);
void _longjmp(jmp_buf env, int val);

int setjmp(jmp_buf env);
int sigsetjmp(sigjmp_buf env, int val);
int _setjmp(jmp_buf env);

#endif //_SETJMP_H
