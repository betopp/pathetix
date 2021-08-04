//kentry.c
//Kernel entry points
//Bryan E. Topp <betopp@betopp.com> 2021

#include "con.h"
#include "fd.h"
#include "thread.h"
#include "process.h"
#include "kassert.h"
#include "syscalls.h"
#include "libcstubs.h"

#include "hal_exit.h"

#include "px.h"
#include <errno.h>

//Called on bootstrap core before releasing other cores.
//Should return when single-threaded initialization is finished.
void kentry_boot(void)
{
	//Init kernel and make initial process/thread
	con_init();
	fd_init();
	thread_init();
	process_init();
	
	//Return to start scheduling threads
}

//Called on all cores after kentry_boot. Should never return.
void kentry_sched(void)
{
	thread_sched(); //Should not return.
	KASSERT(0);
}

//Called when user code makes a system-call.
//Should not return. Instead, call hal_exit_resume with the given exit-buffer.
void kentry_syscall(uint64_t call, uint64_t p1, uint64_t p2, uint64_t p3, uint64_t p4, uint64_t p5, hal_exit_t *eptr)
{
	//Perform the system-call as requested and store return-value in the kernel-exit context.
	eptr->vals[HAL_EXIT_IDX_RV] = syscalls_switch(call, p1, p2, p3, p4, p5);
	
	//On return from system-call, check that the calling process should keep executing.
	//If the process is supposed to be exiting, kill the thread instead of returning to userland.
	process_t *pptr = process_lockcur();
	if(pptr->state != PROCESS_STATE_ALIVE)
	{
		process_unlock(pptr);
		process_leave();
		thread_die();
		
		//thread_die shouldn't return
		KASSERT(0);
	}
	
	//Set aside the location of the process's entry point while we're at it.
	uintptr_t process_entry = pptr->entry;
	
	process_unlock(pptr);
	pptr = NULL;
	
	//See if the thread has a pending signal.
	//If so, return to the signal handler instead.
	//Restore any temporarily-saved signal mask, regardless.
	thread_t *tptr = thread_lockcur();
	
	//Get the location of the kernel-stack pointer for the thread, for re-entering the kernel after we leave.
	void *sp = tptr->stack_top;
	
	int64_t sigready = tptr->sigpend & ~(tptr->sigmask_cur);
	tptr->sigmask_cur = tptr->sigmask_ret;
	if(sigready)
	{
		//Find which signal we'll handle
		int sigcaught_num = 0;
		int64_t sigcaught_mask = 1;
		while(!(sigready & sigcaught_mask))
		{
			sigcaught_mask <<= 1;
			sigcaught_num++;
		}
		
		//The signal is no longer pending
		tptr->sigpend &= ~sigcaught_mask;
		
		//Save our exit context to the thread, so it can retrieve it later
		KASSERT(eptr->vals[0] <= sizeof(tptr->sigexit.vals));
		memcpy(tptr->sigexit.vals, eptr->vals, eptr->vals[0]);
		
		//Save signal information for the thread to get
		tptr->siginfo.signum = sigcaught_num;
		tptr->siginfo.sigmask = tptr->sigmask_cur;
		
		//Cause the system-call to return to the entry point for signal handling with all signals blocked
		eptr->vals[HAL_EXIT_IDX_PC] = process_entry + 16;
		tptr->sigmask_cur = ~0ul;
		tptr->sigmask_ret = ~0ul;
	}
	
	thread_unlock(tptr);
	tptr = NULL;
	
	//Return from the system call with the given return value
	hal_exit_resume(eptr, sp);
	KASSERT(0);
}

//Called when an exception is caught by hardware.
//The state of the CPU should be preserved already, and continue if this returns.
void kentry_exception(int signum, uint64_t pc_addr, uint64_t ref_addr, hal_exit_t *eptr)
{
	//Check if this exception was in user-space or kernel-space.
	//For now we don't handle exceptions in the kernel.
	//Todo - we'll need to handle exceptions in the kernel for safely copying to/from userspace in a timely way.
	uintptr_t uspc_start = 0;
	uintptr_t uspc_end = 0;
	hal_uspc_bound(&uspc_start, &uspc_end);
	if(pc_addr < uspc_start || pc_addr >= uspc_end)
	{
		static const char *exc = "exception caught in kernel space";
		con_panic(exc);
		hal_panic(exc);
		while(1) { }
	}
	
	//Exception came from user-space.
	//We'll allow user-space to handle it like a signal being raised.
	
	//Figure out where the signal handler entry is
	process_t *pptr = process_lockcur();
	uintptr_t signal_entry = pptr->entry + 16;
	process_unlock(pptr);
	
	//See if the signal is masked currently
	thread_t *tptr = thread_lockcur();
	int64_t sigcaught_bit = (1ul << signum);
	if(tptr->sigmask_cur & sigcaught_bit)
	{
		//We need to raise a signal to handle the exception, but that signal is masked.
		//There's nothing we can do - the process must die.
		thread_unlock(tptr);
		
		process_t *pptr = process_lockcur();
		if(pptr->state != PROCESS_STATE_EXITING)
		{
			pptr->exitstatus = _WIFSIGNALED_FLAG | ((signum << _WTERMSIG_SHIFT) & _WTERMSIG_MASK);
			pptr->state = PROCESS_STATE_EXITING;
		}		
		process_unlock(pptr);
		
		process_leave();
		thread_die();
		KASSERT(0);
	}
	
	//Save the exit context for when the user code wants to return to the interrupted context
	KASSERT(eptr->vals[0] < sizeof(tptr->sigexit.vals));
	memcpy(tptr->sigexit.vals, eptr->vals, eptr->vals[0]);
	
	//Save signal information for the signal handler to retrieve
	tptr->siginfo.signum = signum;
	tptr->siginfo.sigmask = tptr->sigmask_cur;
	tptr->siginfo.sender = 0;
	tptr->siginfo.instruction = pc_addr;
	tptr->siginfo.referenced = ref_addr;	
	
	//Return instead to the signal handler entry point, with all signals masked
	//(This is how double-faulting kills a program. The next fault will exit in the case above because the signal is masked.)
	eptr->vals[HAL_EXIT_IDX_PC] = signal_entry;
	tptr->sigmask_cur = ~0ul;
	tptr->sigmask_ret = ~0ul;
	
	void *sp = tptr->stack_top;
	thread_unlock(tptr);
	tptr = NULL;
	
	hal_exit_resume(eptr, sp);
	KASSERT(0);
}

//Called in interrupt context when a key is pressed or released on the keyboard.
void kentry_isr_kbd(hal_kbd_scancode_t scancode, bool state)
{
	con_kbd(scancode, state);
}
