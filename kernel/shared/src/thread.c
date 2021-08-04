//thread.c
//Thread management / scheduling
//Bryan E. Topp <betopp@betopp.com> 2021

#include "thread.h"
#include "kspace.h"
#include "process.h"
#include "kassert.h"
#include "libcstubs.h"
#include "hal_intr.h"
#include "hal_ktls.h"
#include "hal_frame.h"

//Thread table
static thread_t *thread_array;
static int thread_count;

//First code executed when switching to new threads, before their entry function.
void thread_preentry(void)
{
	//Get entry function/data pointers from our thread control block
	thread_t *tptr = hal_ktls_get(); //Set up in call to hal_ctx_reset
	void (*entry_func)(void *data) = tptr->entry_func;
	void *entry_data = tptr->entry_data;
	
	//Release thread control block (locked when context-switching in) and execute thread at entry point
	thread_unlock(tptr);
	(*entry_func)(entry_data);
	
	//Threads should kill themselves rather than return here.
	KASSERT(0);
}

void thread_init(void)
{
	int threads = 256; //Todo - allow adjusting with a command line or something
	thread_array = kspace_alloc(sizeof(thread_t) * threads, alignof(thread_t));
	KASSERT(thread_array != NULL);
	thread_count = threads;
}

static thread_t *thread_locknew(void)
{
	for(int tt = 0; tt < thread_count; tt++)
	{
		//Try to lock this array entry, but give up if contested
		if(hal_spl_try(&(thread_array[tt].spl)))
		{
			if(thread_array[tt].state == THREAD_STATE_NONE)
			{
				//Found a free spot.
				//Give the thread an ID that maps to its spot in the table.
				if( (thread_array[tt].id % thread_count) != tt )
					thread_array[tt].id = tt;
				
				//Advance ID every time we use a table entry
				thread_array[tt].id += thread_count;
				
				return &(thread_array[tt]);
			}
			
			//Not free, keep looking
			hal_spl_unlock(&(thread_array[tt].spl));
		}
	}
	
	//No room in thread table
	return NULL;
}

thread_t *thread_new(void (*entry_func)(void *data), void *entry_data)
{
	//Find a spot in the thread table
	thread_t *tptr = thread_locknew();
	if(tptr == NULL)
	{
		//No room for new threads
		return NULL;
	}
	
	//Make stack
	size_t stacksz = hal_frame_size(); //Todo - configurable?
	tptr->stack_bottom = kspace_alloc(stacksz, stacksz);
	if(tptr->stack_bottom == NULL)
	{
		//No room for stack
		hal_spl_unlock(&(tptr->spl));
		return NULL;
	}
	tptr->stack_size = stacksz;
	tptr->stack_top = (void*)( (uint8_t*)(tptr->stack_bottom) + stacksz );
	
	//Set up initial CPU state - to execute "pre-entry" function and complete thread set-up
	hal_ctx_reset(&(tptr->ctx), &thread_preentry, ((uint8_t*)(tptr->stack_bottom)) + stacksz, tptr);
	
	//Store entry pointers for when the thread runs its pre-entry
	tptr->entry_func = entry_func;
	tptr->entry_data = entry_data;
	
	//Make the thread runnable by default
	tptr->state = THREAD_STATE_READY;

	//Return it, still locked
	return tptr;
}

thread_t *thread_lockcur(void)
{
	thread_t *tptr = hal_ktls_get();
	KASSERT(tptr != NULL);
	
	hal_spl_lock(&(tptr->spl));
	return tptr;
}

thread_t *thread_getlocked(id_t tid)
{
	//Don't attempt to look up negative IDs
	if(tid < 0)
		return NULL;
	
	//Thread ID should correspond with position in the array.
	thread_t *tptr = &(thread_array[tid % thread_count]);
	hal_spl_lock(&(tptr->spl));
	if(tptr->id != tid || tptr->state == THREAD_STATE_NONE)
	{
		//Wrong/no thread in this entry
		hal_spl_unlock(&(tptr->spl));
		return NULL;
	}
	
	//Got it, return still locked
	return tptr;
}

void thread_unlock(thread_t *tptr)
{
	hal_spl_unlock(&(tptr->spl));
}

void thread_yield(thread_t *tptr)
{
	//The thread control block should already be locked by us.
	KASSERT(tptr == hal_ktls_get());
	KASSERT(tptr->spl > 0);
	
	//Switch back to the scheduling context.
	//The scheduling context will unlock our thread control block after the switch.
	KASSERT(tptr->sched_ctx != NULL);
	hal_ctx_switch(&(tptr->ctx), tptr->sched_ctx);
	
	//When we're eventually rescheduled, we'll arrive back here.
	//The scheduling context will have locked our thread control block.
	//Leave it locked - as it was when we initially entered.
	//Our caller will unlock it.
}

void thread_die(void)
{
	//The calling thread should already have been removed from its process, if any
	thread_t *tptr = thread_lockcur();
	KASSERT(tptr->process == NULL);
	
	//Set our state to dead and switch away.
	//The scheduling context will clean us up.
	tptr->state = THREAD_STATE_DONE;
	thread_yield(tptr);
	
	//The scheduling context should never switch back to us.
	KASSERT(0);
}

void thread_sendsig(idtype_t idtype, pid_t id, int signum)
{
	//If we've got an ID other than a thread ID, try to resolve it into a thread ID
	if(idtype == P_PID)
	{
		//Find any thread belonging to this process	
		id_t found_tid = -1;
		for(int tt = 0; tt < thread_count; tt++)
		{
			hal_spl_lock(&(thread_array[tt].spl));
			
			if(thread_array[tt].state > THREAD_STATE_NONE && thread_array[tt].state < THREAD_STATE_DONE)
			{
				if(thread_array[tt].process != NULL)
				{
					process_t *pptr = thread_array[tt].process;
					hal_spl_lock(&(pptr->spl));
					
					if(pptr->id == id)
					{
						//Thread belongs to a process with an ID matching the one we're signalling
						found_tid = thread_array[tt].id;
					}
					
					hal_spl_unlock(&(pptr->spl));
				}
			}
			
			hal_spl_unlock(&(thread_array[tt].spl));
			
			//Stop at the first thread we find in the process
			if(found_tid >= 0)
				break;
		}
		
		if(found_tid >= 0)
		{
			idtype = P_TID;
			id = found_tid;
		}
	}
	
	if(idtype != P_TID)
	{
		//Trying to signal an ID other than a thread-ID, but we couldn't resolve it to a thread.
		return;
	}
	
	//Look up the given thread
	thread_t *tptr = thread_getlocked(id);
	if(tptr == NULL)
	{
		//Couldn't find the thread
		return;
	}
	
	//Raise signal and wake thread if sleeping
	tptr->sigpend |= (1<<signum);
	if(tptr->state == THREAD_STATE_NOTIFY)
		tptr->state = THREAD_STATE_READY;
	
	hal_intr_wake();
	
	thread_unlock(tptr);
}

void thread_sched(void)
{
	//Space to store this CPU state when switching away, into a thread.
	hal_ctx_t sched_ctx = {0};
	KASSERT(hal_ctx_size() <= sizeof(sched_ctx)); //Should really define this properly
	
	//Look for threads to schedule
	while(1)
	{
		//Disable interrupts while scheduling - so we're not stuck holding the run-queue spinlock while an ISR runs.
		hal_intr_ei(false);
		
		//Something tells me I should have a run-queue or something.
		//But... my CPU is real fast so I don't give a shit.
		thread_t *tptr = NULL;
		for(int tt = 0; tt < thread_count; tt++)
		{
			hal_spl_lock(&(thread_array[tt].spl));
			if(thread_array[tt].state == THREAD_STATE_READY)
			{
				tptr = &(thread_array[tt]);
				break;
			}
			
			hal_spl_unlock(&(thread_array[tt].spl));
		}
		
		if(tptr == NULL)
		{
			//No threads ready to run. Sleep, and then try again.
			//Anyone who made a thread runnable while we were looking, would have fired a wakeup IPI.
			//We'll catch that immediately on halting with interrupts enabled, if so.
			hal_intr_halt();
			continue;
		}
		
		//Okay, we have a thread to run, and we've already locked it.
		//Switch into that thread to run it, noting where to switch back.
		//The thread will unlock its thread control block after the switch.
		KASSERT(tptr->sched_ctx == NULL);
		tptr->sched_ctx = &sched_ctx;
		
		KASSERT(tptr->state == THREAD_STATE_READY);
		tptr->state = THREAD_STATE_RUN;
		
		hal_ctx_switch(&sched_ctx, &(tptr->ctx));
		
		//Eventually the thread will want to be descheduled.
		//It will lock its thread control block, change its state, and return here with another hal_ctx_switch.
		KASSERT(tptr->sched_ctx == &sched_ctx);
		tptr->sched_ctx = NULL;
		
		KASSERT(tptr->state != THREAD_STATE_RUN);
		KASSERT(tptr->spl > 0);
		
		if(tptr->state == THREAD_STATE_DONE)
		{
			//Thread finished; clean it up.
			
			//It should have removed itself from its process before dieing.
			KASSERT(tptr->process == NULL);
			
			kspace_free(tptr->stack_bottom, tptr->stack_size);
			tptr->stack_bottom = NULL;
			tptr->stack_top = NULL;
			tptr->stack_size = 0;
			
			tptr->entry_func = NULL;
			tptr->entry_data = NULL;
			
			tptr->ctx = (hal_ctx_t){0};
			
			tptr->sigmask_cur = 0;
			tptr->sigmask_ret = 0;
			tptr->sigpend = 0;
			
			tptr->notify_count = 0;
			tptr->notify_last = 0;
			
			memset(&(tptr->siginfo), 0, sizeof(tptr->siginfo));
			memset(&(tptr->sigexit), 0, sizeof(tptr->sigexit));
			
			tptr->state = THREAD_STATE_NONE;
			
			hal_spl_unlock(&(tptr->spl));			
		}
		else
		{
			//Thread still exists.
			hal_spl_unlock(&(tptr->spl));
		}
	}
	
	KASSERT(0);
}

