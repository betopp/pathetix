//process.c
//Process management
//Bryan E. Topp <betopp@betopp.com> 2021

#include "process.h"
#include "kspace.h"
#include "kassert.h"
#include "systar.h"
#include "libcstubs.h"
#include "thread.h"

#include "px.h"
#include "hal_uspc.h"
#include <errno.h>

//Process table
static process_t *process_array;
static size_t process_count;

//Entry for initial process
void process_init_entry(void *data)
{
	(void)data;
	
	//Activate the initial process userspace (should be empty)
	KASSERT(process_array[1].mem != NULL);
	KASSERT(process_array[1].mem->uspc != HAL_USPC_ID_INVALID);
	hal_uspc_activate(process_array[1].mem->uspc);
	
	//Unpack initial TAR file into RAMfs
	systar_unpack();
	
	//Simulate system calls to exec our init process using the normal codepaths.
	extern int k_px_fd_find();
	int root_fd = k_px_fd_find(-1, "/");
	KASSERT(root_fd >= 0);
	
	int bin_fd = k_px_fd_find(root_fd, "bin");
	KASSERT(bin_fd >= 0);
	
	int fd = k_px_fd_find(bin_fd, "pxinit");
	KASSERT(fd >= 0);
	
	extern int k_px_fd_exec();
	k_px_fd_exec(fd, 
		(char*[]){"pxinit", 0}, 
		(char*[]){"PX=1", "PX_BUILDVERSION="BUILDVERSION, "PX_BUILDDATE="BUILDDATE, "PX_BUILDUSER="BUILDUSER, 0}
	);
		
	KASSERT(0);
}

void process_init(void)
{
	//Allocate process table
	size_t procs = 64; //Todo - support configuring this with a command-line or something.
	process_array = kspace_alloc(procs * sizeof(process_t), alignof(process_t));
	KASSERT(process_array != NULL);
	process_count = procs;
	
	//Make the initial process
	process_t *pptr = &(process_array[1]);
	hal_spl_lock(&(pptr->spl));
	
	pptr->state = PROCESS_STATE_ALIVE;
	pptr->id = 1;
	
	pptr->mem = mem_space_new();
	KASSERT(pptr->mem != NULL);
	
	int nfds = 64; //Todo - configurable
	pptr->fd_array = kspace_alloc(nfds * sizeof(pptr->fd_array[0]), alignof(pptr->fd_array[0]));
	KASSERT(pptr->fd_array != NULL);
	pptr->fd_count = nfds;
	
	thread_t *tptr = thread_new(&process_init_entry, pptr);
	tptr->process = pptr;
	pptr->nthreads = 1;
	thread_unlock(tptr);
	
	fd_t *rootpwd = fd_new();
	rootpwd->ino = 0;
	pptr->fd_pwd = rootpwd->id;
	rootpwd->refs = 1;
	fd_unlock(rootpwd);
	
	hal_spl_unlock(&(pptr->spl));
}

process_t *process_lockcur(void)
{
	thread_t *tptr = thread_lockcur();
	process_t *pptr = tptr->process;
	KASSERT(pptr != NULL);
	hal_spl_lock(&(pptr->spl));
	thread_unlock(tptr);
	return pptr;
}

process_t *process_locknew(void)
{
	for(size_t pp = 0; pp < process_count; pp++)
	{
		//Check each entry in the process table in turn. If one is busy just skip it.
		if(hal_spl_try(&(process_array[pp].spl)))
		{
			if(process_array[pp].state == PROCESS_STATE_NONE)
			{
				//Found a free entry. Give it a valid ID - that maps to the entry in the table.
				if( (process_array[pp].id % process_count) != pp )
					process_array[pp].id = pp;
				
				//Each time we use a table entry, advance its ID.
				process_array[pp].id += process_count;
				
				return &(process_array[pp]); //Still locked
			}
			
			//Wasn't free, keep looking
			hal_spl_unlock(&(process_array[pp].spl));
		}
	}
	
	//No room in process table
	return NULL;
}

process_t *process_getlocked(id_t id)
{
	if(id <= 0)
		return NULL;
	
	//The process ID implies where in the table the process could be found.
	//Lock that table entry.
	process_t *pptr = &(process_array[id % process_count]);
	hal_spl_lock(&(pptr->spl));
	
	//See if it's actually the ID we were looking for - or has been reused since, or not used.
	if( (pptr->id != id) || (pptr->state == PROCESS_STATE_NONE) )
	{
		//Not really the process we're looking for
		hal_spl_unlock(&(pptr->spl));
		return NULL;
	}
	
	//Got the process we wanted.
	//Keep it locked and return it.
	return pptr;
}

void process_unlock(process_t *pptr)
{
	hal_spl_unlock(&(pptr->spl));
}

void process_leave(void)
{
	//Switch back to no-userland pagetables
	hal_uspc_activate(HAL_USPC_ID_INVALID);
	
	//Remove the reference from the thread control block
	thread_t *tptr = thread_lockcur();
	process_t *pptr = tptr->process;
	tptr->process = NULL;
	thread_unlock(tptr);
	
	//Remove the reference from the process control block
	hal_spl_lock(&(pptr->spl));
	pptr->nthreads--;
	KASSERT(pptr->nthreads >= 0);
	
	//If there's more threads in the process, that's all. Otherwise, we need to clean it up.
	if(pptr->nthreads > 0)
	{
		process_unlock(pptr);
		return;
	}
	
	//If this was the last thread in the process, the process is now dead.
	//If nobody set the exit-status yet, do it
	if(pptr->state != PROCESS_STATE_EXITING)
		pptr->exitstatus = 0;
	
	//Init process death is fatal to the kernel too
	KASSERT(pptr->id != 1);
	
	//Free all the memory of the process
	mem_space_delete(pptr->mem);
	pptr->mem = NULL;
	
	//Free all file descriptors
	for(int ff = 0; ff < pptr->fd_count; ff++)
	{
		if(pptr->fd_array[ff].id != 0)
		{
			fd_decr(pptr->fd_array[ff].id);
			pptr->fd_array[ff].id = 0;
		}
	}
	
	if(pptr->fd_pwd != 0)
	{
		fd_decr(pptr->fd_pwd);
		pptr->fd_pwd = 0;
	}
	
	//Free the array of file descriptors
	kspace_free(pptr->fd_array, sizeof(pptr->fd_array[0]) * pptr->fd_count);
	pptr->fd_array = NULL;
	pptr->fd_count = 0;
	
	//Indicate that the process is done and just needs to be waited upon.
	pptr->state = PROCESS_STATE_DONE;
	pptr->waitstatus = WEXITED;
	int notify_pid = pptr->parent;
	process_unlock(pptr);
	
	//Fire a notify to threads waiting on child status in that parent
	process_t *parent_pptr = process_getlocked(notify_pid);
	if(parent_pptr != NULL)
	{
		notify_send(&(parent_pptr->child_notify));
		process_unlock(parent_pptr);
	}
	
	//Signal the parent that a child has exited (might go to any arbitrary thread)
	thread_sendsig(P_PID, notify_pid, SIGCHLD);
}

int process_addfd(id_t id, int min, bool overwrite, id_t *old_id)
{
	process_t *pptr = process_lockcur();
	KASSERT(pptr != NULL);
	
	if(min == -1)
	{
		min = 0;
		overwrite = false;
	}
	
	if(min < 0 || min >= pptr->fd_count)
	{
		process_unlock(pptr);
		return -EINVAL;
	}
	
	for(int nn = min; nn < pptr->fd_count; nn++)
	{
		if( (pptr->fd_array[nn].id == 0) || overwrite )
		{
			if(old_id != NULL)
			{
				//Store the ID being replaced, 0 or otherwise, if we have somewhere to
				*old_id = pptr->fd_array[nn].id;
			}
			else
			{
				//If the old ID is being ignored, it better not be nonzero - we'd lose the reference
				KASSERT(pptr->fd_array[nn].id == 0);
			}
			
			pptr->fd_array[nn].id = id;
			pptr->fd_array[nn].flags = 0;
			process_unlock(pptr);
			return nn;
		}
	}
	
	process_unlock(pptr);
	return -EMFILE;	
}

id_t process_getfdnum(int num)
{
	process_t *pptr = process_lockcur();
	KASSERT(pptr != NULL);
	if(num < 0 || num >= pptr->fd_count)
	{
		process_unlock(pptr);
		return 0;
	}
	
	id_t retval = pptr->fd_array[num].id;
	process_unlock(pptr);
	return retval;
}

id_t process_clearfdnum(int num)
{
	process_t *pptr = process_lockcur();
	KASSERT(pptr != NULL);
	if(num < 0 || num >= pptr->fd_count)
	{
		process_unlock(pptr);
		return 0;
	}
	
	id_t retval = pptr->fd_array[num].id;
	pptr->fd_array[num].id = 0;
	pptr->fd_array[num].flags = 0;
	process_unlock(pptr);
	return retval;
}

int process_flagfdnum(int num, int set, int clr)
{
	process_t *pptr = process_lockcur();
	KASSERT(pptr != NULL);
	if(num < 0 || num >= pptr->fd_count)
	{
		process_unlock(pptr);
		return -EBADF;
	}
	
	if(pptr->fd_array[num].id == 0)
	{
		process_unlock(pptr);
		return -EBADF;
	}
	
	pptr->fd_array[num].flags |= set;
	pptr->fd_array[num].flags &= ~clr;
	
	//Restrict to valid flags
	pptr->fd_array[num].flags &= PX_FD_FLAG_KEEPEXEC; // ...| OTHER_FLAG | OTHER_FLAG...
	
	int retval = pptr->fd_array[num].flags;
	process_unlock(pptr);
	KASSERT(retval >= 0);
	return retval;
}

id_t process_getfdpwd(void)
{
	process_t *pptr = process_lockcur();
	KASSERT(pptr != NULL);
	
	id_t retval = pptr->fd_pwd;
	process_unlock(pptr);
	return retval;
}

int process_strncpy_touser(void *dst_u, const void *src_k, size_t buflen)
{
	//Todo - safety...
	strncpy(dst_u, src_k, buflen);
	return 0;
}

//Makes a single attempt to wait for a child process to change status, returning if there's none available.
static int process_wait_attempt(id_t caller_pid, idtype_t id_type, int64_t id, int options, px_wait_t *out)
{
	//See if any processes match the requested ID and have status available.
	process_t *found_pptr = NULL;
	int children = 0;
	for(size_t pp = 0; pp < process_count; pp++)
	{
		process_t *check_pptr = &(process_array[pp]);
		hal_spl_lock(&(check_pptr->spl));
		
		if(check_pptr->state != PROCESS_STATE_NONE && check_pptr->parent == caller_pid)
		{
			children++;	
			if((id_type == P_ALL) || (id_type == P_PID && check_pptr->id == id) || (id_type == P_PGID && check_pptr->pgid == id))
			{
				if(check_pptr->waitstatus & options)
				{
					//Got a match. Keep the lock held and continue with it.
					found_pptr = check_pptr;
					break;
				}
			}
		}
		
		hal_spl_unlock(&(check_pptr->spl)); //Keep looking
	}
	
	if(found_pptr == NULL)
	{
		if(children == 0)
		{
			//No children at all
			return -ECHILD;
		}
		else
		{
			//No children with status
			return -EAGAIN;
		}
	}
	
	//Success
	out->pid = found_pptr->id;
	out->waitst = found_pptr->waitstatus;
	out->exitst = found_pptr->exitstatus;
	found_pptr->waitstatus = 0;
	
	//If we just waited on an otherwise-dead child, it's now fully dead.
	if(found_pptr->state == PROCESS_STATE_DONE)
	{
		//Should have already been mostly cleaned up by this point
		KASSERT(found_pptr->mem == NULL);
		KASSERT(found_pptr->fd_array == NULL);
		found_pptr->state = PROCESS_STATE_NONE;
	}
	
	process_unlock(found_pptr);
	return 0;
}

int process_wait(idtype_t id_type, int64_t id, int options, px_wait_t *out)
{
	//Set up calling process for a wait...
	process_t *caller_pptr = process_lockcur();
	
	//Subscribe this thread to the notify that gets fired when a child changes status
	notify_dst_t n = {0};
	notify_add(&(caller_pptr->child_notify), &n);

	//Get ID of caller for checking parentage / group IDs
	pid_t caller_pid = caller_pptr->id;
	pid_t caller_pgid = caller_pptr->pgid;
	if( (id == 0) && ((id_type == P_PID) || (id_type == P_PGID)) )
	{
		id = caller_pgid;
		id_type = P_PGID;
	}
	
	process_unlock(caller_pptr);
	caller_pptr = NULL;
	
	//Repeatedly check for available status, waiting if none is found.
	int result;
	while(1)
	{
		//Check for any matching children with status changes
		result = process_wait_attempt(caller_pid, id_type, id, options, out);
		if(result != -EAGAIN)
			break; //Success or total failure - something besides "wait on it"
		
		//Alright, we have children but none have status changes.
		if(options & WNOHANG)
			break; //Don't want to try again
		
		//Wait for notifies on this thread.
		//This includes the child_notify in our owning process, which gets sent when its children change status.
		result = notify_wait();
		if(result < 0)
			break; //Probably got interrupted
		
		//Try again after we're woken up
		continue;
	}
	
	//Unsubscribe from notifications for our process having children with status-changes
	caller_pptr = process_lockcur();
	
	notify_remove(&(caller_pptr->child_notify), &n);
	
	process_unlock(caller_pptr);
	caller_pptr = NULL;
	
	return result;
}
