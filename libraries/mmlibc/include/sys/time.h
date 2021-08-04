//mmlibc/include/sys/time.h
//Time-related declarations for MMK's libc.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _SYS_TIME_H
#define _SYS_TIME_H

#include <mmbits/struct_timeval.h>
#include <mmbits/struct_itimerval.h>
#include <mmbits/typedef_time.h>
#include <mmbits/typedef_suseconds.h>
#include <mmbits/typedef_fdset.h>
#include <mmbits/define_itimer_options.h>
#include <mmbits/define_fdset_funcs.h>

int getitimer(int which, struct itimerval *value);
int gettimeofday(struct timeval *tp, void *tzp);
int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *errorfds, struct timeval *timeout);
int setitimer(int which, const struct itimerval *value, struct itimerval *ovalue);
int utimes(const char *path, const struct timeval times[2]);

#endif // _SYS_TIME_H
