//notify.c
//Sloppy thread wakeups
//Bryan E. Topp <betopp@betopp.com> 2021

#include "notify.h"
#include "kassert.h"
#include "thread.h"
#include "errno.h"
#include "hal_intr.h"

#include <stddef.h>

void notify_add(notify_src_t *src, notify_dst_t *dst)
{
	thread_t *tptr = thread_lockcur();
	dst->next = src->dsts;
	dst->tid = tptr->id;
	src->dsts = dst;
	thread_unlock(tptr);
}

void notify_remove(notify_src_t *src, notify_dst_t *dst)
{
	notify_dst_t **dd = &(src->dsts);
	while(*dd != dst && *dd != NULL)
		dd = &((*dd)->next);
	
	KASSERT(*dd != NULL);
	*dd = (*dd)->next;
}

int notify_wait(void)
{
	while(1)
	{
		//Inspect the current thread's control block, to see if we want to return yet.
		thread_t *tptr = thread_lockcur();
		if(tptr->sigpend & ~tptr->sigmask_cur)
		{
			//We caught a signal. Ignore whether we got notified or not - caller should bail for signal handling.
			thread_unlock(tptr);
			return -EINTR;
		}
		else if(tptr->notify_count > tptr->notify_last)
		{
			//Got a notify since the last time. We're done waiting. Caller should check their conditions again.
			tptr->notify_last = tptr->notify_count;
			thread_unlock(tptr);
			return 0;
		}
		else
		{
			//Neither signalled nor notified. Wait.
			tptr->state = THREAD_STATE_NOTIFY;
			thread_yield(tptr); //Unlocks the thread control block, but then re-locks when we are scheduled once more.
			thread_unlock(tptr);
			continue;
		}
	}
}

void notify_send(notify_src_t *src)
{
	for(notify_dst_t *dd = src->dsts; dd != NULL; dd = dd->next)
	{		
		thread_t *tptr = thread_getlocked(dd->tid);
		if(tptr == NULL)
			continue;
		
		tptr->notify_count++;
		if(tptr->state == THREAD_STATE_NOTIFY)
			tptr->state = THREAD_STATE_READY;
		
		hal_intr_wake();
		
		thread_unlock(tptr);
	}
}
