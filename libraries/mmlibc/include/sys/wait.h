//mmlibc/include/sys/wait.h
//Waiting-related declarations for MMK's libc.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _SYS_WAIT_H
#define _SYS_WAIT_H

#include <mmbits/define_waitpid_options.h>
#include <mmbits/define_wait_macros.h>
#include <mmbits/define_waitid_options.h>
#include <mmbits/typedef_idtype.h>
#include <mmbits/typedef_id.h>
#include <mmbits/typedef_pid.h>
#include <mmbits/typedef_siginfo.h>
#include <mmbits/union_sigval.h>

pid_t wait(int *stat_loc);
int waitid(idtype_t idtype, id_t id, siginfo_t *infop, int options);
pid_t waitpid(pid_t pid, int *stat_loc, int options);

#endif // _SYS_WAIT_H
