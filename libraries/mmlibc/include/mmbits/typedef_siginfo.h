//mmlibc/include/mmbits/typedef_siginfo.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _TYPEDEF_SIGINFO_H
#define _TYPEDEF_SIGINFO_H

#include <mmbits/union_sigval.h>

typedef struct _siginfo_s
{
	int si_signo; //Signal number
	int si_code; //Signal code
	int si_errno; //If nonzero, an errno value associated with the signal
	pid_t si_pid; //Sending process ID
	uid_t si_uid; //Real user-ID of sending process
	void *si_addr; //Address of faulting instruction
	int si_status; //Exit value or signal
	long si_band; //Band event for SIGPOLL
	union sigval si_value; //Signal value
	
} siginfo_t;

#endif //_TYPEDEF_SIGINFO_H


