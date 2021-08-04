//mmlibc/include/limits.h
//Type limit functions for MMK's libc
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _LIMITS_H
#define _LIMITS_H

//for the value of FOPEN_MAX, even though we don't define FOPEN_MAX in this header.
#include <mmbits/define_fopen_max.h>

//Standards-compliance minimum values
#define _POSIX_AIO_LISTIO_MAX               2
#define _POSIX_AIO_MAX                      1
#define _POSIX_ARG_MAX                      4096
#define _POSIX_CHILD_MAX                    25
#define _POSIX_DELAYTIMER_MAX               32
#define _POSIX_HOST_NAME_MAX                255
#define _POSIX_LINK_MAX                     8
#define _POSIX_LOGIN_NAME_MAX               9
#define _POSIX_MAX_CANON                    255
#define _POSIX_MAX_INPUT                    255
#define _POSIX_MQ_OPEN_MAX                  8
#define _POSIX_MQ_PRIO_MAX                  32
#define _POSIX_NAME_MAX                     14
#define _POSIX_NGROUPS_MAX                  8
#define _POSIX_OPEN_MAX                     20
#define _POSIX_PATH_MAX                     256
#define _POSIX_PIPE_BUF                     512
#define _POSIX_RE_DUP_MAX                   255
#define _POSIX_RTSIG_MAX                    8
#define _POSIX_SEM_NSEMS_MAX                256
#define _POSIX_SEM_VALUE_MAX                32767
#define _POSIX_SIGQUEUE_MAX                 32
#define _POSIX_SSIZE_MAX                    32767
#define _POSIX_SS_REPL_MAX                  4
#define _POSIX_STREAM_MAX                   8
#define _POSIX_SYMLINK_MAX                  255
#define _POSIX_SYMLOOP_MAX                  8
#define _POSIX_THREAD_DESTRUCTOR_ITERATIONS 4
#define _POSIX_THREAD_KEYS_MAX              128
#define _POSIX_THREAD_THREADS_MAX           64
#define _POSIX_TIMER_MAX                    32
#define _POSIX_TRACE_EVENT_NAME_MAX         30
#define _POSIX_TRACE_NAME_MAX               8
#define _POSIX_TRACE_SYS_MAX                8
#define _POSIX_TRACE_USER_EVENT_MAX         32
#define _POSIX_TTY_NAME_MAX                 9
#define _POSIX_TZNAME_MAX                   6
#define _POSIX2_BC_BASE_MAX                 99
#define _POSIX2_BC_DIM_MAX                  2048
#define _POSIX2_BC_SCALE_MAX                99
#define _POSIX2_BC_STRING_MAX               1000
#define _POSIX2_CHARCLASS_NAME_MAX          14
#define _POSIX2_COLL_WEIGHTS_MAX            2
#define _POSIX2_EXPR_NEST_MAX               32
#define _POSIX2_LINE_MAX                    2048
#define _POSIX2_RE_DUP_MAX                  255
#define _XOPEN_IOV_MAX                      16
#define _XOPEN_NAME_MAX                     255
#define _XOPEN_PATH_MAX                     1024
#define _POSIX_CLOCKRES_MIN                 20000000

//Runtime increasable values
#define BC_BASE_MAX        _POSIX2_BC_BASE_MAX
#define BC_DIM_MAX         _POSIX2_BC_DIM_MAX
#define BC_SCALE_MAX       _POSIX2_BC_SCALE_MAX
#define BC_STRING_MAX      _POSIX2_BC_STRING_MAX
#define CHARCLASS_NAME_MAX _POSIX2_CHARCLASS_NAME_MAX
#define COLL_WEIGHTS_MAX   _POSIX2_COLL_WEIGHTS_MAX
#define EXPR_NEST_MAX      _POSIX2_EXPR_NEST_MAX
#define LINE_MAX           _POSIX2_LINE_MAX
#define NGROUPS_MAX        _POSIX_NGROUPS_MAX
#define RE_DUP_MAX         _POSIX_RE_DUP_MAX

//Runtime invariant values
#define AIO_LISTIO_MAX                _POSIX_AIO_LISTIO_MAX
#define AIO_MAX                       _POSIX_AIO_MAX
#define AIO_PRIO_DELTA_MAX            0
#define ARG_MAX                       _POSIX_ARG_MAX
#define ATEXIT_MAX                    32
#define CHILD_MAX                     _POSIX_CHILD_MAX
#define DELAYTIMER_MAX                _POSIX_DELAYTIMER_MAX
#define HOST_NAME_MAX                 _POSIX_HOST_NAME_MAX
#define IOV_MAX                       _XOPEN_IOV_MAX
#define LOGIN_NAME_MAX                _POSIX_LOGIN_NAME_MAX
#define MQ_OPEN_MAX                   _POSIX_MQ_OPEN_MAX
#define MQ_PRIO_MAX                   _POSIX_MQ_PRIO_MAX
#define OPEN_MAX                      _POSIX_OPEN_MAX
#define PAGESIZE                      4096
#define PAGE_SIZE                     PAGESIZE
#define PTHREAD_DESTRUCTOR_ITERATIONS _POSIX_THREAD_DESTRUCTOR_ITERATIONS
#define PTHREAD_KEYS_MAX              _POSIX_THREAD_KEYS_MAX
#define PTHREAD_STACK_MIN             0
#define PTHREAD_THREADS_MAX           _POSIX_THREAD_THREADS_MAX
#define RTSIG_MAX                     _POSIX_RTSIG_MAX
#define SEM_NSEMS_MAX                 _POSIX_SEM_NSEMS_MAX
#define SEM_VALUE_MAX                 _POSIX_SEM_VALUE_MAX
#define SIGQUEUE_MAX                  _POSIX_SIGQUEUE_MAX
#define SS_REPL_MAX                   _POSIX_SS_REPL_MAX
#define STREAM_MAX                    _FOPEN_MAX //see define_fopen_max.h
#define SYMLOOP_MAX                   _POSIX_SYMLOOP_MAX
#define TIMER_MAX                     _POSIX_TIMER_MAX
#define TRACE_EVENT_NAME_MAX          _POSIX_TRACE_EVENT_NAME_MAX
#define TRACE_NAME_MAX                _POSIX_TRACE_NAME_MAX
#define TRACE_SYS_MAX                 _POSIX_TRACE_SYS_MAX
#define TRACE_USER_EVENT_MAX          _POSIX_TRACE_USER_EVENT_MAX
#define TTY_NAME_MAX                  _POSIX_TTY_NAME_MAX
#define TZNAME_MAX                    _POSIX_TZNAME_MAX

//Pathname variable values
#define FILESIZEBITS             64
#define LINK_MAX                 _POSIX_LINK_MAX
#define MAX_CANON                _POSIX_MAX_CANON
#define MAX_INPUT                _POSIX_MAX_INPUT
#define NAME_MAX                 _XOPEN_NAME_MAX
#define PATH_MAX                 _XOPEN_PATH_MAX
#define PIPE_BUF                 _POSIX_PIPE_BUF
#define POSIX_ALLOC_SIZE_MIN     1
#define POSIX_REC_INCR_XFER_SIZE 4096
#define POSIX_REC_MAX_XFER_SIZE  1048576
#define POSIX_REC_MIN_XFER_SIZE  512
#define SYMLINK_MAX              _POSIX_SYMLINK_MAX

//Numerical limits
#include <mmbits/define_char_bit.h>
#include <mmbits/define_long_bit.h>
#include <mmbits/define_word_bit.h>

#include <mmbits/limits_char.h>
#include <mmbits/limits_shrt.h>
#include <mmbits/limits_long.h>
#include <mmbits/limits_llong.h>
#include <mmbits/limits_int.h>
#include <mmbits/limits_ssize.h>

//Other invariant values
#define NL_ARGMAX  9
#define NL_LANGMAX 14
#define NL_MSGMAX  32767
#define NL_SETMAX  255
#define NL_TEXTMAX _POSIX2_LINE_MAX
#define NZERO      20

#endif //_LIMITS_H
