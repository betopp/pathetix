//mmlibc/include/sys/resource.h
//Resource operations for MMK's libc
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _SYS_RESOURCE_H
#define _SYS_RESOURCE_H

#include <mmbits/define_prio_options.h>
#include <mmbits/typedef_rlim.h>
#include <mmbits/define_rlim_values.h>
#include <mmbits/define_rusage_options.h>
#include <mmbits/struct_rlimit.h>
#include <mmbits/struct_rusage.h>
#include <mmbits/struct_timeval.h>
#include <mmbits/define_rlimit_options.h>
#include <mmbits/typedef_id.h>

int getpriority(int which, id_t who);
int getrlimit(int resource, struct rlimit *rlp);
int getrusage(int who, struct rusage *r_usage);
int setpriority(int which, id_t who, int value);
int setrlimit(int resource, const struct rlimit *rlp);

#endif //_SYS_RESOURCE_H
