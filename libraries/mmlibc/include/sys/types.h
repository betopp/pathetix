//mmlibc/include/sys/types.h
//Type definitions for MMK's libc
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _SYS_TYPES_H
#define _SYS_TYPES_H

#include <mmbits/typedef_blkcnt.h>
#include <mmbits/typedef_blksize.h>
#include <mmbits/typedef_clock.h>
#include <mmbits/typedef_clockid.h>
#include <mmbits/typedef_dev.h>

typedef unsigned long fsblkcnt_t;
typedef unsigned long fsfilcnt_t;

#include <mmbits/typedef_gid.h>
#include <mmbits/typedef_id.h>
#include <mmbits/typedef_ino.h>

typedef unsigned long key_t;

#include <mmbits/typedef_mode.h>
#include <mmbits/typedef_nlink.h>
#include <mmbits/typedef_off.h>
#include <mmbits/typedef_pid.h>
#include <mmbits/typedef_pthread_attr.h>
#include <mmbits/typedef_pthread_barrier.h>
#include <mmbits/typedef_pthread_barrierattr.h>
#include <mmbits/typedef_pthread_cond.h>
#include <mmbits/typedef_pthread_condattr.h>
#include <mmbits/typedef_pthread_key.h>
#include <mmbits/typedef_pthread_mutex.h>
#include <mmbits/typedef_pthread_mutexattr.h>
#include <mmbits/typedef_pthread_once.h>
#include <mmbits/typedef_pthread_rwlock.h>
#include <mmbits/typedef_pthread_rwlockattr.h>
#include <mmbits/typedef_pthread_spinlock.h>
#include <mmbits/typedef_pthread.h>
#include <mmbits/typedef_size.h>
#include <mmbits/typedef_ssize.h>
#include <mmbits/typedef_suseconds.h>
#include <mmbits/typedef_time.h>
#include <mmbits/typedef_timer.h>
#include <mmbits/typedef_uid.h>

#endif //_SYS_TYPES_H
