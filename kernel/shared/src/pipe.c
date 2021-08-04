//pipe.c
//Pipe implementation in kernel
//Bryan E. Topp <betopp@betopp.com> 2021

#include "pipe.h"
#include "hal_spl.h"
#include "kassert.h"
#include "kspace.h"
#include "notify.h"
#include "libcstubs.h"

#include <stddef.h>
#include <errno.h>
#include <px.h>



//States a pipe can be in
typedef enum pipe_state_e
{
	PIPE_STATE_NONE = 0,
	PIPE_STATE_ALIVE,
	PIPE_STATE_MAX
} pipe_state_t;	

//Information about a pipe.
typedef struct pipe_s
{
	//Spinlock protecting the pipe
	hal_spl_t spl;
	
	//State of pipe
	pipe_state_t state;
	
	//Reference count of overall references
	int refs;
	
	//Reference count of readers
	int refs_r;
	
	//Reference count of writers
	int refs_w;
	
	//ID of the pipe
	int64_t id;
	
	//Data buffer of pipe
	char *buf_ptr;
	size_t buf_len;
	
	//Next byte to read and write from pipe. next_r == next_w implies the pipe is empty.
	size_t next_r;
	size_t next_w;
	
	//Notification fired when the pipe is read or written
	notify_src_t notify;
	
} pipe_t;

//All pipes, who cares
#define PIPE_MAX 4096
static pipe_t pipe_array[PIPE_MAX];

//Returns how many more bytes can be read from the given pipe's buffer before it is empty.
static size_t pipe_canread(const pipe_t *p)
{
	size_t r = p->next_r;
	size_t w = p->next_w;
	if(w < r)
		w += p->buf_len;
	
	return w - r;
}

//Returns how many bytes can be read in a single shot from the pipe's buffer (without wrapping).
static size_t pipe_canread_single(const pipe_t *p)
{
	size_t to_buf = p->buf_len - p->next_r;
	size_t to_wptr = pipe_canread(p);
	
	return (to_wptr < to_buf) ? to_wptr : to_buf;
}

//Returns how many more bytes can be written to the given pipe's buffer before it is full.
static size_t pipe_canwrite(const pipe_t *p)
{
	size_t r = p->next_r;
	size_t w = p->next_w;
	if(w < r)
		w += p->buf_len;
	
	return (p->buf_len - 1) + r - w;
}

//Returns how many bytes can be written in a single shot to the pipe's buffer (without wrapping).
static size_t pipe_canwrite_single(const pipe_t *p)
{
	size_t to_buf = p->buf_len - p->next_w;
	size_t to_rptr = pipe_canwrite(p);
	
	return (to_rptr < to_buf) ? to_rptr : to_buf;
}



static pipe_t *pipe_locknew(void)
{
	for(int pp = 0; pp < PIPE_MAX; pp++)
	{
		pipe_t *pptr = &(pipe_array[pp]);
		if(hal_spl_try(&(pptr->spl)))
		{
			if(pptr->state == PIPE_STATE_NONE)
			{
				//Found a free spot. Give it a new ID that maps to this location in the array.
				if(pptr->id == 0)
					pptr->id = pp;
				
				pptr->id += PIPE_MAX;
				
				return pptr;
			}
			hal_spl_unlock(&(pptr->spl)); //Keep looking
		}
	}
	
	//No free spots
	return NULL;
}

static pipe_t *pipe_getlocked(id_t id)
{
	//Don't try to look up negative IDs
	if(id < 0)
		return NULL;
	
	//Lock the array entry corresponding to this ID
	pipe_t *pptr = &(pipe_array[id % PIPE_MAX]);
	hal_spl_lock(&(pptr->spl));
	
	if(pptr->id != id || pptr->state == PIPE_STATE_NONE)
	{
		//No pipe / wrong pipe in this spot
		hal_spl_unlock(&(pptr->spl));
		return NULL;
	}
	
	//Got it, return still locked
	return pptr;
}

static void pipe_unlock(pipe_t *pptr)
{
	KASSERT(pptr->refs >= 0); //Should never go negative
	if(pptr->refs == 0)
	{		
		//Last reference lost, pipe is gone
		KASSERT(pptr->refs_r == 0); //read/write counts should have been decremented before this
		KASSERT(pptr->refs_w == 0);
		KASSERT(pptr->buf_ptr != NULL);
		KASSERT(pptr->buf_len > 0);
		kspace_free(pptr->buf_ptr, pptr->buf_len);
		pptr->buf_ptr = NULL;
		pptr->buf_len = 0;
		
		pptr->state = PIPE_STATE_NONE;
	}
	
	hal_spl_unlock(&(pptr->spl));
}

id_t pipe_new(void)
{
	pipe_t *pptr = pipe_locknew();
	if(pptr == NULL)
		return -ENFILE;
	
	KASSERT(pptr->buf_ptr == NULL);
	pptr->buf_ptr = kspace_alloc(65536, 65536);
	if(pptr->buf_ptr == NULL)
	{
		pipe_unlock(pptr);
		return -ENOMEM;
	}
	pptr->buf_len = 65536;
	
	pptr->next_r = 0;
	pptr->next_w = 0;
	
	pptr->state = PIPE_STATE_ALIVE;
	pptr->refs = 1;
	
	id_t retval = pptr->id;
	pipe_unlock(pptr);
	return retval;
}

int pipe_incr(id_t id, int access)
{
	pipe_t *pptr = pipe_getlocked(id);
	if(pptr == NULL)
		return -EBADF;
	
	if(access == 0)
	{
		pptr->refs++;
		KASSERT(pptr->refs > 0);
	}
	else
	{
		if(access & PX_FD_ACCESS_R)
		{
			pptr->refs_r++;
			KASSERT(pptr->refs_r > 0);
		}
		
		if(access & PX_FD_ACCESS_W)
		{
			pptr->refs_w++;
			KASSERT(pptr->refs_w > 0);
		}
	}	
	
	pipe_unlock(pptr);
	return 0;
}

int pipe_decr(id_t id, int access)
{
	pipe_t *pptr = pipe_getlocked(id);
	if(pptr == NULL)
		return -EBADF;
	
	if(access == 0)
	{
		pptr->refs--;
		KASSERT(pptr->refs >= 0);
	}
	else
	{
		if(access & PX_FD_ACCESS_R)
		{
			pptr->refs_r--;
			KASSERT(pptr->refs_r >= 0);
		}
		
		if(access & PX_FD_ACCESS_W)
		{
			pptr->refs_w--;
			KASSERT(pptr->refs_w >= 0);
		}
	}
	
	pipe_unlock(pptr); //Handles deletion if refs == 0
	return 0;
}

ssize_t pipe_write(int64_t id, const void *buf, size_t nbytes)
{
	pipe_t *pptr = pipe_getlocked(id);
	if(pptr == NULL)
		return -EBADF;
	
	//Wait for pipe to be ready, as necessary
	while(pipe_canwrite(pptr) < 512) //Ensure that writes of 512 bytes or less will complete atomically (POSIX req)
	{
		if(pptr->refs_r <= 0)
		{
			//Pipe has no space to write and no readers.
			KASSERT(pptr->refs_r == 0);
			pipe_unlock(pptr);
			return -EPIPE; //Broken pipe for write with no reader; contrast read with no writer.
		}
		
		notify_dst_t n = {0};
		notify_add(&(pptr->notify), &n);
		
		pipe_unlock(pptr);
		int wait_err = notify_wait();
		pptr = pipe_getlocked(id);
		if(pptr == NULL)
			return -EBADF; //Pipe disappeared while we were waiting...? (Is this a valid case?)
		
		notify_remove(&(pptr->notify), &n);
		
		if(wait_err < 0)
		{
			pipe_unlock(pptr);
			return wait_err;
		}
	}
	
	//Write as much as we can
	const char *buf_bytes = (const char*)buf;
	ssize_t written = 0;
	while(1)
	{
		size_t copylen = pipe_canwrite_single(pptr);
		if(copylen > nbytes)
			copylen = nbytes;
		
		if(copylen == 0)
			break;
		
		memcpy(pptr->buf_ptr + pptr->next_w, buf_bytes, copylen);
		pptr->next_w = (pptr->next_w + copylen) % (pptr->buf_len);
		
		buf_bytes += copylen;
		written += copylen;
		nbytes -= copylen;
	}
	
	notify_send(&(pptr->notify));
	pipe_unlock(pptr);
	return written;
}

ssize_t pipe_read(int64_t id, void *buf, size_t nbytes)
{
	pipe_t *pptr = pipe_getlocked(id);
	if(pptr == NULL)
		return -EBADF;
	
	//Wait for pipe to be ready, as necessary
	while(pipe_canread(pptr) < 1)
	{
		if(pptr->refs_w <= 0)
		{
			//Pipe has no data to read and no writers.
			KASSERT(pptr->refs_w == 0);
			pipe_unlock(pptr);
			return 0; //EOF for read with no writer; contrast write with no reader.
		}
		
		notify_dst_t n = {0};
		notify_add(&(pptr->notify), &n);
		
		pipe_unlock(pptr);
		int wait_err = notify_wait();
		pptr = pipe_getlocked(id);
		if(pptr == NULL)
			return -EBADF; //Pipe disappeared while we were waiting...? (Is this a valid case?)
		
		notify_remove(&(pptr->notify), &n);
		
		if(wait_err < 0)
		{
			pipe_unlock(pptr);
			return wait_err;
		}
	}
	
	//Read as much as we can
	char *buf_bytes = (char*)buf;
	ssize_t nread = 0;
	while(1)
	{
		size_t copylen = pipe_canread_single(pptr);
		if(copylen > nbytes)
			copylen = nbytes;
		
		if(copylen == 0)
			break;
		
		memcpy(buf_bytes, pptr->buf_ptr + pptr->next_r, copylen);
		pptr->next_r = (pptr->next_r + copylen) % (pptr->buf_len);
		
		buf_bytes += copylen;
		nread += copylen;
		nbytes -= copylen;
	}
	
	notify_send(&(pptr->notify));
	pipe_unlock(pptr);
	return nread;
}

