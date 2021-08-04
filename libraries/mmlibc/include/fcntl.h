//mmlibc/include/fcntl.h
//File control definitions for MMK's libc
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _FCNTL_H
#define _FCNTL_H

#include <mmbits/define_fcntl_cmds.h>
#include <mmbits/define_fcntl_fdflags.h>
#include <mmbits/define_fcntl_lock.h>

#include <mmbits/define_whence_options.h>

#include <mmbits/define_oflags.h>

#include <mmbits/typedef_mode.h>
#include <mmbits/typedef_off.h>
#include <mmbits/typedef_pid.h>

#include <mmbits/define_modes.h>

#include <mmbits/define_atflags.h>

#include <mmbits/struct_flock.h>

int creat(const char *path, mode_t mode);
int fcntl(int fildes, int cmd, ...);
int open(const char *pathname, int flags, ...);
int openat(int dirfd, const char *pathname, int flags, ...);

#endif //_FCNTL_H
