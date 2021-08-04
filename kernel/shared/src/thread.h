//thread.h
//Thread management / scheduling
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef THREAD_H
#define THREAD_H

#include "hal_spl.h"
#include "hal_ctx.h"
#include "hal_exit.h"

#include <sys/wait.h>

#include "px.h"

//States a thread can be in
typedef enum thread_state_e
{
	THREAD_STATE_NONE = 0, //Thread does not exist
	THREAD_STATE_READY, //Thread can be scheduled
	THREAD_STATE_RUN, //Thread is currently executing
	THREAD_STATE_NOTIFY, //Blocked waiting for notify or signal
	THREAD_STATE_DONE, //Thread can be cleaned up
	THREAD_STATE_MAX
	
} thread_state_t;

//Thread control block
typedef struct thread_s
{
	//Spinlock protecting the thread control block
	hal_spl_t spl;
	
	//ID of this thread
	int id;
	
	//State of this thread control block
	thread_state_t state;
	
	//Kernel context of this thread, stored when switching away
	hal_ctx_t ctx;
	
	//Kernel space allocated for stack
	void *stack_bottom;
	void *stack_top;
	size_t stack_size;
	
	//Function to run when initially entering the thread
	void (*entry_func)(void *data);
	void *entry_data;
	
	//Context to return to on deschedule, if running
	hal_ctx_t *sched_ctx;
	
	//Process that owns this thread
	struct process_s *process;
	
	//Signals currently blocked (bitmask, 1 = blocked)
	int64_t sigmask_cur;
	
	//Old signal mask if the signal mask has been temporarily changed.
	//Restored on return from system-call.
	int64_t sigmask_ret;
	
	//Signals currently pending (bitmask, 1 = pending)
	int64_t sigpend;
	
	//Information about last signal taken
	px_siginfo_t siginfo;
	
	//User return state saved when a signal is taken, to be used on exiting the signal handler
	hal_exit_t sigexit;
	
	//Notifies sent to this thread, total count
	int64_t notify_count;
	
	//Notifies already consumed - notify_count at the last time a wait-for-notify returned.
	int64_t notify_last;
	
} thread_t;

//Initializes thread table
void thread_init(void);

//Makes a new thread to execute the given kernel function.
//Returns a pointer to the thread control block, still locked.
thread_t *thread_new(void (*entry_func)(void *data), void *entry_data);

//Locks and returns the calling thread's thread control block.
thread_t *thread_lockcur(void);

//Locks and returns a thread by ID.
thread_t *thread_getlocked(id_t tid);

//Unlocks the given thread control block.
void thread_unlock(thread_t *tptr);

//Deschedules the calling thread.
//The calling thread should already hold the lock on its thread control block.
//The thread will be descheduled and then unlocked. 
//When the thread is scheduled again, this will return with the thread control block locked once again.
void thread_yield(thread_t *tptr);

//Ends the execution of the calling thread.
//The thread must have already been removed from its process.
void thread_die(void);

//Sends a signal to a thread, by process-ID, thread-ID, etc
void thread_sendsig(idtype_t idtype, pid_t pid, int signum);


//Schedules threads forever. Does not return.
void thread_sched(void);

#endif //THREAD_H
