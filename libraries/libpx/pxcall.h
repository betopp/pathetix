//pxcall.h
//Pathetix system call table
//Bryan E. Topp <betopp@betopp.com> 2021

//This file is used with the macro-trick to define per-system-call entry points.
//PXCALLnR is used for calls that return a value; PXCALLnV is for void returns.
//The first parameter is the call number, then the return type, then the name.
//Parameter types follow.

PXCALL2V(0x01, void,     px_exit,       int, int)

PXCALL0R(0x02, pid_t,    px_getpid)
PXCALL0R(0x03, pid_t,    px_getppid)
PXCALL1R(0x04, pid_t,    px_getpgid,    pid_t)
PXCALL2R(0x05, int,      px_setpgid,    pid_t, pid_t)

PXCALL2R(0x11, int,      px_fd_find,    int, const char *)
PXCALL3R(0x13, ssize_t,  px_fd_read,    int, void *, size_t)
PXCALL3R(0x14, ssize_t,  px_fd_write,   int, const void *, size_t)
PXCALL3R(0x15, off_t,    px_fd_seek,    int, off_t, int)
PXCALL4R(0x16, int,      px_fd_create,  int, const char *, mode_t, uint64_t)
PXCALL3R(0x17, ssize_t,  px_fd_stat,    int, px_fd_stat_t *, size_t)
PXCALL1R(0x18, int,      px_fd_close,   int)
PXCALL3R(0x19, int,      px_fd_exec,    int, char * const *, char * const *)
PXCALL3R(0x1A, int,      px_fd_dup,     int, int, bool)
PXCALL4R(0x1B, int,      px_fd_ioctl,   int, uint64_t, void *, size_t)
PXCALL3R(0x1C, int,      px_fd_access,  int, int, int)
PXCALL3R(0x1D, int,      px_fd_flag,    int, int, int)
PXCALL2R(0x1E, int,      px_fd_trunc,   int, off_t)
PXCALL4R(0x1F, int,      px_fd_unlink,  int, const char *, int, int)

PXCALL1R(0x30, int,      px_chdir,      int)

PXCALL3R(0x20, int,      px_setrlimit,  int, const px_rlimit_t *, size_t)
PXCALL3R(0x21, int,      px_getrlimit,  int, px_rlimit_t *, size_t)
PXCALL3R(0x22, int,      px_rusage,     int, px_rusage_t *, size_t)

PXCALL2R(0x40, int64_t,  px_sigmask,    int, int64_t)
PXCALL1R(0x41, int,      px_sigsuspend, int64_t)
PXCALL3R(0x42, int,      px_sigsend,    idtype_t, int64_t, int)
PXCALL2R(0x43, ssize_t,  px_siginfo,    px_siginfo_t *, size_t)
PXCALL0R(0x44, int,      px_sigexit)

PXCALL0R(0x50, int64_t,  px_getrtc)
PXCALL1R(0x51, int,      px_setrtc,     int64_t)
PXCALL1R(0x52, int,      px_nanosleep,  int64_t)
PXCALL4R(0x53, int64_t,  px_timer_set,  timer_t, int, int64_t, int64_t)
PXCALL1R(0x54, int64_t,  px_timer_get,  timer_t)

PXCALL1R(0x60, pid_t,    px_fork,       uintptr_t)
PXCALL5R(0x61, ssize_t,  px_wait,       idtype_t, int64_t, int, px_wait_t *, size_t)
PXCALL3R(0x62, int,      px_priority,   idtype_t, int64_t, int)

PXCALL2R(0x70, intptr_t, px_mem_avail,  uintptr_t, size_t)
PXCALL3R(0x71, int,      px_mem_anon,   uintptr_t, size_t, int)
