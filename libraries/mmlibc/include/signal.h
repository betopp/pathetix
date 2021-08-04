//mmlibc/include/signal.h
//Signal handling declarations for MMK's libc
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _SIGNAL_H
#define _SIGNAL_H

#include <mmbits/define_sighandling.h>
#include <mmbits/typedef_pthread.h>
#include <mmbits/typedef_size.h>
#include <mmbits/typedef_uid.h>
#include <mmbits/struct_timespec.h>
#include <mmbits/typedef_sigatomic.h>
#include <mmbits/typedef_sigset.h>
#include <mmbits/typedef_pid.h>
#include <mmbits/typedef_pthread_attr.h>
#include <mmbits/struct_sigevent.h>
#include <mmbits/define_sigevoptions.h>
#include <mmbits/union_sigval.h>

//Like BSD.
#define SIGZERO 0
#define SIGHUP 1
#define SIGINT 2
#define SIGQUIT 3
#define SIGILL 4
#define SIGTRAP 5
#define SIGABRT 6
#define SIGEMT 7
#define SIGFPE 8
#define SIGKILL 9
#define SIGBUS 10
#define SIGSEGV 11
#define SIGSYS 12
#define SIGPIPE 13
#define SIGALRM 14
#define SIGTERM 15
#define SIGURG 16
#define SIGSTOP 17
#define SIGTSTP 18
#define SIGCONT 19
#define SIGCHLD 20
#define SIGTTIN 21
#define SIGTTOU 22
#define SIGXCPU 24
#define SIGXFSZ 25
#define SIGVTALRM 26
#define SIGPROF 27
#define SIGWINCH 28
#define SIGINFO 29
#define SIGUSR1 30
#define SIGUSR2 31
#define SIGTHR 32

#define SIGRTMIN 33
#define SIGRTMAX 62

//Total signals are 63, so we can report a 63-bit mask and still pack into a positive int64_t.
#define _SIG_MAX 63

#include <mmbits/struct_sigaction.h>
#include <mmbits/define_sig_options.h>
#include <mmbits/define_sa_options.h>

#include <mmbits/typedef_mcontext.h>
#include <mmbits/typedef_ucontext.h>
#include <mmbits/typedef_stack.h>
#include <mmbits/typedef_siginfo.h>

//Like Linux.
#define ILL_ILLOPC 1
#define ILL_ILLOPN 2
#define ILL_ILLADR 3
#define ILL_ILLTRP 4
#define ILL_PRVOPC 5
#define ILL_PRVREG 6
#define ILL_COPROC 7
#define ILL_BADSTK 8
#define FPE_INTDIV 1
#define FPE_INTOVF 2
#define FPE_FLTDIV 3
#define FPE_FLTOVF 4
#define FPE_FLTUND 5
#define FPE_FLTRES 6
#define FPE_FLTINV 7
#define FPE_FLTSUB 8
#define SEGV_MAPERR 1
#define SEGV_ACCERR 2
#define BUS_ADRALN 1
#define BUS_ADRERR 2
#define BUS_OBJERR 3
#define CLD_EXITED 1
#define CLD_KILLED 2
#define CLD_DUMPED 3
#define CLD_TRAPPED 4
#define CLD_STOPPED 5
#define CLD_CONTINUED 6
#define POLL_IN 1
#define POLL_OUT 2
#define POLL_MSG 3
#define POLL_ERR 4
#define POLL_PRI 5
#define POLL_HUP 6
#define SI_USER 0
#define SI_QUEUE (-1)
#define SI_TIMER (-2)
#define SI_ASYNCIO (-4)
#define SI_MESGQ (-3)

//GNU and BSD both define this to make the declarations of signal() less awful...
typedef void (*_sighandler_t)(int);

int kill(pid_t pid, int sig);
int killpg(pid_t pgrp, int sig);
void psiginfo(const siginfo_t *pinfo, const char *s);
void psignal(int sig, const char *s);
int pthread_kill(pthread_t thread, int sig);
int pthread_sigmask(int how, const sigset_t *set, sigset_t *oldset);
int raise(int sig);
int sigaction(int sig, const struct sigaction *act, struct sigaction *oact);
int sigaddset(sigset_t *set, int signum);
int sigaltstack(const stack_t *ss, stack_t *oss);
int sigdelset(sigset_t *set, int signum);
int sigemptyset(sigset_t *set);
int sigfillset(sigset_t *set);
int sighold(int sig);
int sigignore(int sig);
int siginterrupt(int sig, int flag);
int sigismember(const sigset_t *set, int signum);
_sighandler_t signal(int signum, _sighandler_t handler);
//int sigpause(int sig); //Dumb historical problems; just avoid this.
int sigpending(sigset_t *set);
int sigprocmask(int how, const sigset_t *set, sigset_t *oset);
int sigqueue(pid_t pid, int sig, const union sigval value);
int sigrelse(int sig);
_sighandler_t sigset(int sig, _sighandler_t disp);
int sigsuspend(const sigset_t *sigmask);
int sigtimedwait(const sigset_t *set, siginfo_t *info, const struct timespec *timeout);
int sigwait(const sigset_t *set, int *sig);
int sigwaitinfo(const sigset_t *set, siginfo_t *info);

extern const char * const sys_siglist[];
extern const char * const sys_signame[];

#endif //_SIGNAL_H
