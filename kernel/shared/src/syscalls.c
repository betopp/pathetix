//syscalls.c
//System call implementation
//Bryan E. Topp <betopp@betopp.com> 2021

#include <errno.h>
#include <sys/stat.h>
#include <sys/resource.h>

#include "px.h"

#include "hal_exit.h"

#include "fd.h"
#include "libcstubs.h"
#include "process.h"
#include "thread.h"
#include "kspace.h"
#include "kassert.h"
#include "elf64.h"
#include "argenv.h"
#include "notify.h"


//Big todo - these need some kind of safety so they can be aborted when accessing userspace.
//We need to ensure that a bad userspace address doesn't cause an unhandled exception in the kernel.
//In the simplest case we could just have a copy_to_userspace function and do them all in kernel-mode.


int k_px_fd_find(int at, const char *name)
{
	//Todo - validate memory and copy name to kernelspace safely
	char kname[256] = {0};
	strncpy(kname, name, 255);
	
	//Make the new file descriptor and start it with one reference
	id_t newid = 0;

	if(strcmp(kname, "/") != 0)
	{
		if(strchr(kname, '/') != NULL)
		{
			//Pathname component contains a slash, and is not simply "/"
			return -EINVAL;
		}
	}
	
	//Find where we'll look it up
	id_t at_id = 0;
	if(at < 0)
	{
		at_id = process_getfdpwd();
	}
	else
	{
		at_id = process_getfdnum(at);
	}
	
	if(at_id == 0)
	{
		//Directory to search is invalid
		return -EBADF;
	}
	
	newid = fd_find(at_id, kname); //Will give us the FD with one reference already
	if(newid < 0)
	{
		//Error looking up the name
		return newid;
	}

	
	//Store the reference to the file descriptor in the calling process.
	KASSERT(newid > 0);
	int newnum = process_addfd(newid, 0, false, NULL);
	if(newnum < 0)
	{
		//Failed to add the new file descriptor... take away its reference, as we didn't actually store it
		fd_decr(newid);
	}
	
	//Return the file descriptor number in the calling process.
	return newnum;
}

int k_px_fd_access(int fd, int set, int clr)
{
	id_t id = process_getfdnum(fd);
	if(id == 0)
		return -EBADF;
	
	return fd_access(id, set, clr);
}

int k_px_fd_flag(int fd, int set, int clr)
{
	return process_flagfdnum(fd, set, clr);
}

ssize_t k_px_fd_read(int fd, void *buf, size_t len)
{
	id_t id = process_getfdnum(fd);
	if(id == 0)
		return -EBADF;
	
	return fd_read(id, buf, len);
}

ssize_t k_px_fd_write(int fd, const void *buf, size_t len)
{
	id_t id = process_getfdnum(fd);
	if(id == 0)
		return -EBADF;
	
	return fd_write(id, buf, len);
}

off_t k_px_fd_seek(int fd, off_t off, int whence)
{
	id_t id = process_getfdnum(fd);
	if(id == 0)
		return -EBADF;

	return fd_seek(id, off, whence);
}

int k_px_fd_create(int at, const char *name, mode_t mode, uint64_t spec)
{
	id_t id = process_getfdnum(at);
	if(id == 0)
		return -EBADF;
	
	//Make sure the mode is sane. Todo - probably need more caution.
	bool any_type = S_ISCHR(mode) | S_ISDIR(mode) | S_ISREG(mode) | S_ISFIFO(mode);
	if(!any_type)
		return -EINVAL;
	
	//Todo - validate memory and copy name to kernelspace safely
	char kname[256] = {0};
	strncpy(kname, name, 255);
	
	id_t newid = fd_create(id, kname, mode, spec);
	if(newid < 0)
	{
		//Failed to make the file
		return newid;
	}
		
	
	//Store the reference to the file descriptor in the calling process.
	int newnum = process_addfd(newid, 0, false, NULL);
	if(newnum < 0)
	{
		//Failed to add the new file descriptor... take away its reference, as we didn't actually store it
		fd_decr(newid);
	}
	
	//Return the file descriptor number in the calling process.
	return newnum;
}

ssize_t k_px_fd_stat(int fd, px_fd_stat_t *buf, size_t len)
{
	id_t id = process_getfdnum(fd);
	if(id == 0)
		return -EBADF;
	
	return fd_stat(id, buf, len);
}

int k_px_fd_trunc(int fd, off_t size)
{
	id_t id = process_getfdnum(fd);
	if(id == 0)
		return -EBADF;
	
	return fd_trunc(id, size);
}

int k_px_fd_unlink(int at, const char *name, int onlyfd, int rmdir)
{
	id_t at_id = process_getfdnum(at);
	if(at_id == 0)
		return -EBADF;
	
	id_t only_id = 0;
	if(onlyfd >= 0)
	{
		only_id = process_getfdnum(onlyfd);
		if(only_id == 0)
			return -EBADF;
	}
	
	return fd_unlink(at_id, name, only_id, rmdir);
}

int k_px_fd_close(int fd)
{
	id_t id = process_clearfdnum(fd);
	if(id == 0)
		return -EBADF;
	
	fd_decr(id);
	return 0;
}

int k_px_fd_exec(int fd, char * const *argv, char * const *envp)
{	
	//Error code returned on failure (success never returns)
	int err_ret = 0;
	
	//New memory space we'll be setting up (has to be cleaned up on failure)
	mem_space_t *new_mem = NULL;
	
	//Look up file descriptor we'll be exec()'ing
	id_t id = process_getfdnum(fd);
	if(id == 0)
	{
		err_ret = -EBADF; //Executable file descriptor not found
		goto failure;
	}
	
	//Make new userspace and memory-map info
	new_mem = mem_space_new();
	if(new_mem == NULL)
	{
		err_ret = -ENOMEM; //No memory for memory space info
		goto failure;
	}
	
	//Try to load the given file
	uintptr_t entry = 0;
	int elf_err = elf64_load(id, new_mem, &entry);
	if(elf_err < 0)
	{
		err_ret = elf_err; //Failed to load ELF file
		goto failure;
	}
	
	//Try to place the argv/envp in the new memory image for the crt0 to find.
	int argenv_err = argenv_load(new_mem, argv, envp);
	if(argenv_err < 0)
	{
		err_ret = argenv_err; //Failed to insert argv/envp into new process
		goto failure;
	}
	
	//Alright, we're about to start destroying the old process image.
	//Any possible failures after this point would be very bad and wreck the process if they occur.
	process_t *pptr = process_lockcur();
	KASSERT(pptr != NULL);
	
	//Todo - make sure all other threads in this process are dead
		
	KASSERT(pptr->nthreads == 1); //Should only be this thread at the moment.
	
	//Switch over to the new memory space before deleting the old one
	hal_uspc_activate(new_mem->uspc);
	
	//Get rid of old memory space of the process, including private frames
	KASSERT(pptr->mem != NULL);
	mem_space_delete(pptr->mem);
	pptr->mem = NULL;
	
	//Put the new image in its place....
	pptr->mem = new_mem;
	pptr->entry = entry;
	
	//This thread drops to userspace to be the initial thread in the new program.
	//Reset the kernel-stack pointer on the return to kernel-mode.
	thread_t *tptr = thread_lockcur();
	void *sp = tptr->stack_top;
	thread_unlock(tptr);
	
	//Drop to execute the new user-mode code.
	process_unlock(pptr);
	hal_exit_fresh(entry, sp);
	
	//Should never get here
	KASSERT(0);
	
failure:
	if(new_mem != NULL)
	{
		mem_space_delete(new_mem);
		new_mem = NULL;
	}
	
	KASSERT(err_ret < 0);
	return err_ret;
}

int k_px_fd_dup(int oldfd, int newmin, bool overwrite)
{
	//Get the descriptor behind the old number and add a reference to it (so it can't disappear while adding)
	id_t id = process_getfdnum(oldfd);
	if(id == 0)
		return -EBADF;
	
	int64_t refs = fd_incr(id);
	if(refs <= 0)
		return -EBADF; 
	
	//Store it as specified, and figure out if we just overwrote a reference to something else.
	id_t old_id = 0;
	int result = process_addfd(id, newmin, overwrite, &old_id);
	if(result < 0)
	{
		//Failed to add the reference - restore original reference-count
		fd_decr(id);
		return result;
	}
	
	//Alright, stored the new reference.
	//If we overwrote some old reference, change its reference-count too.
	if(old_id != 0)
	{
		fd_decr(old_id);
	}
	
	return result;
}

int k_px_chdir(int fd)
{
	//Get the file descriptor that this number refers to, and increment its reference count.
	//Do this first so it doesn't disappear from under us, if another thread closes the file.
	id_t new_pwd = process_getfdnum(fd);
	if(new_pwd == 0)
		return -EBADF;
	
	int64_t incr_err = fd_incr(new_pwd);
	if(incr_err < 0)
		return incr_err;
	
	//Replace the reference in the process, setting aside the old one atomically
	process_t *pptr = process_lockcur();
	id_t old_pwd = pptr->fd_pwd;
	pptr->fd_pwd = new_pwd;
	process_unlock(pptr);
	
	//The old descriptor now has one less reference
	fd_decr(old_pwd);
	
	return 0;
}

int k_px_exit(int status, int signal)
{	
	//Lock the calling process and set it to exiting with the given status.
	//This will cause all its threads to exit rather than returning to userland.
	process_t *pptr = process_lockcur();
	if(pptr->state != PROCESS_STATE_EXITING)
	{
		pptr->exitstatus = 0;
		pptr->exitstatus |= _WIFEXITED_FLAG;
		pptr->exitstatus |= status & 0xFF;
		
		if(signal > 0 && signal < 63)
		{
			pptr->exitstatus |= _WIFSIGNALED_FLAG;
			pptr->exitstatus |= (signal << _WTERMSIG_SHIFT) & _WTERMSIG_MASK;
		}
		
		pptr->state = PROCESS_STATE_EXITING;
	}
	process_unlock(pptr);
	
	//Pull this thread out of the process.
	//When all threads pull out of the process, the process is dead.
	process_leave();
	
	//Terminate the calling thread now that it's out of the process.
	thread_die();
	
	//Should never return here
	KASSERT(0);
	return -ENOSYS;
}

int k_px_fd_ioctl(int fd, uint64_t request, void *ptr, size_t len)
{
	id_t id = process_getfdnum(fd);
	if(id == 0)
		return -EBADF;
	
	return fd_ioctl(id, request, ptr, len);
}

pid_t k_px_getpid(void)
{
	process_t *pptr = process_lockcur();
	int id = pptr->id;
	process_unlock(pptr);
	return id;
}

pid_t k_px_getppid(void)
{
	process_t *pptr = process_lockcur();
	int id = pptr->parent;
	process_unlock(pptr);
	return id;
}

pid_t k_px_getpgid(pid_t pid)
{
	process_t *pptr = NULL;
	if(pid == 0)
		pptr = process_lockcur();
	else
		pptr = process_getlocked(pid);
	
	if(pptr == NULL)
		return -ESRCH;
		
	pid_t retval = pptr->pgid;
	process_unlock(pptr);
	return retval;
}

int k_px_setpgid(pid_t pid, pid_t pgrp)
{	
	//If pgrp is 0, this is interpreted to mean "the group of the calling process".
	if(pgrp == 0)
	{
		process_t *pptr_cur = process_lockcur();
		pgrp = pptr_cur->pgid;
		process_unlock(pptr_cur);
	}
	
	process_t *pptr = NULL;
	if(pid == 0)
		pptr = process_lockcur();
	else
		pptr = process_getlocked(pid);
	
	if(pptr == NULL)
		return -ESRCH;	
	
	pptr->pgid = pgrp;
	process_unlock(pptr);
	return 0;
}

int k_px_setrlimit(int resource, const px_rlimit_t *ptr, size_t len)
{
	if(resource < 0 || resource >= _RLIMIT_MAX)
		return -EINVAL;
	
	//Retrieve parameter from userspace - careful about size for forward-compatibility
	px_rlimit_t lim = {0};
	if(len > sizeof(px_rlimit_t))
		len = sizeof(px_rlimit_t);
	
	memcpy(&lim, ptr, len);
	
	//Current limit can never exceed max limit
	if( (lim.cur > lim.max) || (lim.cur == RLIM_INFINITY && lim.max != RLIM_INFINITY) )
		return -EINVAL;
	
	process_t *pptr = process_lockcur();
	
	//Max limit cannot exceed prior max limit
	rlim_t old_max = pptr->rlimits[resource].rlim_max;
	if( (lim.max > old_max) || (lim.max == RLIM_INFINITY && old_max != RLIM_INFINITY) )
	{
		process_unlock(pptr);
		return -EPERM;
	}
	
	pptr->rlimits[resource].rlim_max = lim.max;
	pptr->rlimits[resource].rlim_cur = lim.cur;
	
	process_unlock(pptr);
	return 0;
}

int k_px_getrlimit(int resource, px_rlimit_t *ptr, size_t len)
{
	if(resource < 0 || resource >= _RLIMIT_MAX)
		return -EINVAL;
	
	//Get limit
	process_t *pptr = process_lockcur();
	
	px_rlimit_t lim = {0};
	lim.cur = pptr->rlimits[resource].rlim_cur;
	lim.max = pptr->rlimits[resource].rlim_max;
	
	process_unlock(pptr);
	
	//Return result to userspace
	if(len > sizeof(px_rlimit_t))
		len = sizeof(px_rlimit_t);
	
	memcpy(ptr, &lim, len);
	
	return len;
}

int k_px_rusage(int who, px_rusage_t *ptr, size_t len)
{
	//Stub
	(void)who;
	px_rusage_t r = {0};
	
	if(len > sizeof(px_rusage_t))
		len = sizeof(px_rusage_t);
	
	memcpy(ptr, &r, len);
	return 0;
}

int64_t k_px_sigmask(int how, int64_t val)
{
	thread_t *tptr = thread_lockcur();
	
	int64_t oldval = tptr->sigmask_cur;
	switch(how)
	{
		case SIG_BLOCK:
			tptr->sigmask_cur |= val;
			break;
		case SIG_UNBLOCK:
			tptr->sigmask_cur &= ~val;
			break;
		case SIG_SETMASK:
			tptr->sigmask_cur = val;
			break;
		default:
			thread_unlock(tptr);
			return -EINVAL;
	}
	
	tptr->sigmask_cur &= ~((1l << 63) | (1l << SIGKILL) | (1l << SIGSTOP));
	tptr->sigmask_ret = tptr->sigmask_cur; //Persists across return from syscall
	thread_unlock(tptr);
	
	KASSERT(oldval >= 0);
	return oldval;
}

int k_px_sigsuspend(int64_t tempmask)
{
	//Unblock the given signals and wait for any to occur.
	//Change sigmask_cur but not sigmask_ret - so the mask gets restored after this call.
	thread_t *tptr = thread_lockcur();
	tptr->sigmask_cur = tempmask;
	thread_unlock(tptr);
	
	//We didn't set ourselves up to be notified of anything.
	//So discard any notifies that occur - wait specifically for a signal (notify_wait returns -EINTR).
	while(1)
	{
		int notified = notify_wait();
		if(notified < 0)
			return notified;
	}
}

int k_px_sigsend(idtype_t to_type, int64_t to_id, int sig)
{
	thread_sendsig(to_type, to_id, sig);
	return 0;
}

ssize_t k_px_siginfo(px_siginfo_t *out_ptr, size_t out_len)
{
	thread_t *tptr = thread_lockcur();
	if(out_len > sizeof(px_siginfo_t))
		out_len = sizeof(px_siginfo_t);
	
	memcpy(out_ptr, &(tptr->siginfo), out_len);
	thread_unlock(tptr);
	return out_len;
}

int k_px_sigexit(void)
{
	thread_t *tptr = thread_lockcur();
	
	//First word of exit-buffer is its length.
	//If it's 0, we don't have anything to exit to.
	if(tptr->sigexit.vals[0] == 0)
	{
		thread_unlock(tptr);
		return -ESRCH;
	}
	
	//Restore the signal mask at the time of the signal
	tptr->sigmask_cur = tptr->siginfo.sigmask;
	tptr->sigmask_ret = tptr->siginfo.sigmask;
	
	//Clear old signal info
	memset(&(tptr->siginfo), 0, sizeof(tptr->siginfo));
	
	//Copy exit-buffer out of the thread and onto this stack
	hal_exit_t buf = {0};
	KASSERT(tptr->sigexit.vals[0] <= sizeof(buf)); 
	memcpy(&buf, &(tptr->sigexit), tptr->sigexit.vals[0]);
	
	//Clear the exit from the thread as we're consuming it
	memset(&(tptr->sigexit), 0, sizeof(tptr->sigexit));
	
	//With the exit-buffer on the stack, unlock the thread's control block and return to the point it was signalled.
	void *sp = tptr->stack_top;
	thread_unlock(tptr);
	hal_exit_resume(&buf, sp);
	
	//hal_exit_resume shouldn't return
	KASSERT(0);
	return -ENOSYS;
}


int64_t k_px_getrtc(void)
{
	return -ENOSYS;
}

int k_px_setrtc(int64_t val)
{
	(void)val;
	return -ENOSYS;
}

void postfork(void *data)
{
	//Activate the proper userspace for the new process
	//Note that this waits until k_px_fork is done, because they lock our process until then.
	process_t *pptr = process_lockcur();
	hal_uspc_activate(pptr->mem->uspc);
	process_unlock(pptr);
	
	thread_t *tptr = thread_lockcur();
	void *sp = tptr->stack_top;
	thread_unlock(tptr);
	
	//Drop to the requested userspace address
	hal_exit_fresh((uintptr_t)data, sp);
}

pid_t k_px_fork(uintptr_t child_entry_pc)
{
	//Error returned on failure
	pid_t err_ret = 0;
	
	//Find room for a new process
	process_t *new_pptr = process_locknew();
	if(new_pptr == NULL)
		return -EAGAIN;
	
	//We'll duplicate the calling process and thread.
	//Lock them both.
	process_t *old_pptr = process_lockcur();
	thread_t *old_tptr = thread_lockcur();
	
	//Set up IDs and basic properties of the new process
	KASSERT(new_pptr->id > 0); //process_locknew sets this
	new_pptr->state = PROCESS_STATE_ALIVE;
	new_pptr->parent = old_pptr->id;
	new_pptr->pgid = old_pptr->pgid;
	new_pptr->entry = old_pptr->entry;
	
	//Allocate new memory space with copy of old memory space
	new_pptr->mem = mem_space_fork(old_pptr->mem);
	if(new_pptr->mem == NULL)
	{
		err_ret = -ENOMEM;
		goto failure;
	}
	
	//Allocate array for file descriptors
	KASSERT(new_pptr->fd_array == NULL);
	new_pptr->fd_array = kspace_alloc(sizeof(new_pptr->fd_array[0]) * old_pptr->fd_count, alignof(new_pptr->fd_array[0]));
	if(new_pptr->fd_array == NULL)
	{
		err_ret = -ENOMEM;
		goto failure;
	}
	new_pptr->fd_count = old_pptr->fd_count;

	//Start a thread for the new process.
	//It helps that this is the last failure case - so we don't have to kill a thread that started.
	thread_t *newthread = thread_new(postfork, (void*)child_entry_pc);
	if(newthread == NULL)
	{
		err_ret = -ENOMEM;
		goto failure;
	}
	
	newthread->process = new_pptr;
	new_pptr->nthreads = 1;
	
	//Copy over file descriptor numbers.
	//There should be no failure here - file descriptors shouldn't disappear while we lock a process that still refers to them.
	if(old_pptr->fd_pwd != 0)
	{
		new_pptr->fd_pwd = old_pptr->fd_pwd;
		int incr_err = fd_incr(new_pptr->fd_pwd);
		KASSERT(incr_err >= 0);
	}
	
	for(int ff = 0; ff < old_pptr->fd_count; ff++)
	{
		if(old_pptr->fd_array[ff].id != 0)
		{
			new_pptr->fd_array[ff] = old_pptr->fd_array[ff];
			int incr_err = fd_incr(old_pptr->fd_array[ff].id);
			KASSERT(incr_err >= 0);
		}
	}
	
	//Copy resource limits
	memcpy(new_pptr->rlimits, old_pptr->rlimits, sizeof(new_pptr->rlimits));
	
	//Unlock the new process so the new thread can get it (see: postfork)
	pid_t retval = new_pptr->id;
	process_unlock(new_pptr);
	thread_unlock(newthread);
	
	//Done with our calling process/thread
	process_unlock(old_pptr);
	thread_unlock(old_tptr);
	
	//Return ID of process created
	return retval;
	
failure:
		
	//Free userspace
	if(new_pptr->mem != NULL)
	{
		mem_space_delete(new_pptr->mem);
		new_pptr->mem = NULL;
	}
	
	//Free file descriptor array
	//Note - we don't currently have a failure case where we have to decrement FD ref counts.
	if(new_pptr->fd_array != NULL)
	{
		KASSERT(new_pptr->fd_count > 0);
		
		for(int ff = 0; ff < new_pptr->fd_count; ff++)
		{
			KASSERT(new_pptr->fd_array[ff].id == 0);
		}
		
		kspace_free(new_pptr->fd_array, sizeof(new_pptr->fd_array[0]) * new_pptr->fd_count);
		new_pptr->fd_array = NULL;
		new_pptr->fd_count = 0;
	}
	
	new_pptr->state = PROCESS_STATE_NONE;
	process_unlock(new_pptr);
	process_unlock(old_pptr);
	thread_unlock(old_tptr);
	
	KASSERT(err_ret < 0);
	return err_ret;
}

int k_px_nanosleep(int64_t ns)
{
	(void)ns;
	return -EINTR;
}

ssize_t k_px_wait(idtype_t id_type, int64_t id, int options, px_wait_t *ptr, size_t len)
{
	//Wait, putting the result in a kernel-space buffer
	px_wait_t buf = {0};
	int result = process_wait(id_type, id, options, &buf);
	if(result < 0)
		return result;
	
	//Success - copy the result to the user buffer
	if(len > sizeof(buf))
		len = sizeof(buf);
	
	memcpy(ptr, &buf, len);
	return len;
}

int k_px_priority(idtype_t id_type, int64_t id, int prval)
{
	(void)id_type;
	(void)id;
	(void)prval;
	return -ENOSYS;
}

int64_t k_px_timer_set(timer_t id, int flags, int64_t value_ns, int64_t interval_ns)
{
	(void)id;
	(void)flags;
	(void)value_ns;
	(void)interval_ns;
	return -ENOSYS;
}

int64_t k_px_timer_get(timer_t id)
{
	(void)id;
	return -ENOSYS;
}

intptr_t k_px_mem_avail(uintptr_t around, size_t size)
{
	process_t *pptr = process_lockcur();
	intptr_t retval = mem_space_avail(pptr->mem, around, size);
	process_unlock(pptr);
	return retval;
}

int k_px_mem_anon(uintptr_t start, size_t size, int prot)
{
	//Until somebody gives me a good reason otherwise, user processes are not allowed to make executable pages.
	if(prot & PX_MEM_X)
		return -EPERM;
	
	if(prot & ~(PX_MEM_R | PX_MEM_W | PX_MEM_X))
		return -EINVAL;
	
	process_t *pptr = process_lockcur();
	int retval = mem_space_add(pptr->mem, start, size, prot);
	process_unlock(pptr);
	return retval;
}

uint64_t syscalls_switch(uint64_t call, uint64_t p1, uint64_t p2, uint64_t p3, uint64_t p4, uint64_t p5)
{
	//Use macro-trick to call the appropriate function based on system-call number.
	//Declare functions with a k_ prefix on the kernel side, to distinguish from libpx entry points.
	switch(call)
	{
		#define PXCALL0R(num, rt, name)\
			case num: return k_##name();
		
		#define PXCALL0V(num, rt, name)\
			case num: k_##name(); return 0;
		
		#define PXCALL1R(num, rt, name, p1t)\
			case num: return k_##name((p1t)p1);
		
		#define PXCALL1V(num, rt, name, p1t)\
			case num: k_##name((p1t)p1); return 0;
		
		#define PXCALL2R(num, rt, name, p1t, p2t)\
			case num: return k_##name((p1t)p1, (p2t)p2);
		
		#define PXCALL2V(num, rt, name, p1t, p2t)\
			case num: k_##name((p1t)p1, (p2t)p2); return 0;
		
		#define PXCALL3R(num, rt, name, p1t, p2t, p3t)\
			case num: return k_##name((p1t)p1, (p2t)p2, (p3t)p3);
		
		#define PXCALL3V(num, rt, name, p1t, p2t, p3t)\
			case num: k_##name((p1t)p1, (p2t)p2, (p3t)p3); return 0;	
			
		#define PXCALL4R(num, rt, name, p1t, p2t, p3t, p4t)\
			case num: return k_##name((p1t)p1, (p2t)p2, (p3t)p3, (p4t)p4);
		
		#define PXCALL4V(num, rt, name, p1t, p2t, p3t, p4t)\
			case num: k_##name((p1t)p1, (p2t)p2, (p3t)p3, (p4t)p4); return 0;
			
		#define PXCALL5R(num, rt, name, p1t, p2t, p3t, p4t, p5t)\
			case num: return k_##name((p1t)p1, (p2t)p2, (p3t)p3, (p4t)p4, (p5t)p5);
		
		#define PXCALL5V(num, rt, name, p1t, p2t, p3t, p4t, p5t)\
			case num: k_##name((p1t)p1, (p2t)p2, (p3t)p3, (p4t)p4, (p5t)p5); return 0;
			
		#include "../../../libraries/libpx/pxcall.h"
		
		default:
			return -ENOSYS;
	}
	
	//All of the above cases should return
	KASSERT(0);
}
