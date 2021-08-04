//fork.c
//Process creation in libc
//Bryan E. Topp <betopp@betopp.com> 2021

#include <unistd.h>
#include <errno.h>
#include <px.h>

pid_t fork(void)
{
	//Pathetix kernel doesn't actually support returning from the fork() call on the child.
	//It starts a new thread that begins execution at a given code address with all registers undefined.
	//Simulate fork's return on the child by saving/loading context across the fork.
	
	//This works like setjmp/longjmp.
	extern int _forkctx_save(); //Saves context - returns 0 when saving or 1 when loaded.
	extern void _forkctx_load(); //Loads the context from _forkctx_save.
	
	volatile int ctx = _forkctx_save();
	
	if(ctx == 0)
	{
		// _forkctx_save saved the context. we're on the parent.
		int val = px_fork((uintptr_t)(&_forkctx_load));
		if(val < 0)
		{
			//On parent, error
			errno = -val;
			return -1;
		}
		
		//On parent, success
		return val; 
	}
	else
	{
		// _forkctx_save returned again after _forkctx_load loaded the context. we're on the child.
		return 0;
	}
}
