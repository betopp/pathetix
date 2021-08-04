//mmlibc/include/mmbits/define_wait_macros.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _DEFINE_WAIT_MACROS_H
#define _DEFINE_WAIT_MACROS_H

//Macros that POSIX wants defined to inspect exit status
#define WEXITSTATUS(x)  ((x & 0x000000FF) >>  0)
#define WSTOPSIG(x)     ((x & 0x0000FF00) >>  8)
#define WTERMSIG(x)     ((x & 0x00FF0000) >> 16)
#define WIFCONTINUED(x) ((x & 0x01000000) >> 24)
#define WIFEXITED(x)    ((x & 0x02000000) >> 25)
#define WIFSIGNALED(x)  ((x & 0x04000000) >> 26)
#define WIFSTOPPED(x)   ((x & 0x08000000) >> 27)
#define WCOREDUMP(x)    ((x & 0x10000000) >> 28)

//Helpers we use in libc/kernel for making status values
#define _WIFEXITED_FLAG 0x02000000
#define _WIFSIGNALED_FLAG 0x04000000
#define _WTERMSIG_MASK 0x00FF0000
#define _WTERMSIG_SHIFT 16
#define _WCOREDUMP_FLAG 0x10000000

#endif //_DEFINE_WAIT_MACROS_H

