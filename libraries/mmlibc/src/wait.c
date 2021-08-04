//wait.c
//Child status functions in libc
//Bryan E. Topp <betopp@betopp.com> 2021

#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <px.h>

pid_t wait(int *status)
{
	return waitpid(-1, status, 0);
}

pid_t waitpid(pid_t wpid, int *status, int options)
{
	idtype_t wait_idtype = P_ALL;
	int64_t wait_id = 0;
	if(wpid == -1)
	{
		wait_idtype = P_ALL;
		wait_id = 0;
	}
	else if(wpid == 0)
	{
		wait_idtype = P_PGID;
		wait_id = getpgrp();
	}
	else if(wpid > 0)
	{
		wait_idtype = P_PID;
		wait_id = wpid;
	}
	else if(wpid < -1)
	{
		wait_idtype = P_PGID;
		wait_id = -wpid;
	}
	
	options |= WEXITED;
	
	px_wait_t w = {0};
	ssize_t result = px_wait(wait_idtype, wait_id, options, &w, sizeof(w));
	if(result < 0)
	{
		errno = -result;
		return -1;
	}
	
	//Kernel returns a separate "exit code" value and a "waited flags" value.
	//Build packed status to return.
	int status_ret = 0;
	status_ret |= w.exitst; //Exit code from program
	
	if(w.waitst & WEXITED) //Reason they got waited upon, as kernel sees it
		status_ret |= _WIFEXITED_FLAG;
	
	*status = status_ret;
	return w.pid;
}

int nice(int incr)
{	
	int old = px_priority(P_PID, -1, -1);
	if(old < 0)
	{
		errno = -old;
		return -1;
	}
	
	int newval = old + incr;
	if(newval < 0)
		newval = 0;
	if(newval > 99)
		newval = 99;
	
	int result = px_priority(P_PID, -1, newval);
	if(result < 0)
	{
		errno = -result;
		return -1;
	}
	
	return 0;
}
