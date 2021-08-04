//process.h
//Process management
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef PROCESS_H
#define PROCESS_H

#include "hal_spl.h"
#include "hal_uspc.h"
#include "fd.h"
#include "mem.h"
#include "notify.h"

#include <sys/resource.h>

//State of a process
typedef enum process_state_e
{
	PROCESS_STATE_NONE = 0, //None/empty array entry
	PROCESS_STATE_ALIVE, //Normal state for a process
	PROCESS_STATE_EXITING, //Process wants to end, but has threads still outstanding
	PROCESS_STATE_DONE, //All threads have died, process yet to be waited on
	PROCESS_STATE_MAX
} process_state_t;

//Information about a region of memory in a process
typedef struct process_mem_s
{
	//Address of start of mapped region
	uintptr_t addr;
	
	//Size of mapped region
	size_t size;
	
} process_mem_t;

//Information about a file descriptor reference by a process
typedef struct process_fdnum_s
{
	//Global identity of file descriptor referenced
	id_t id;
	
	//Flags about this reference (px_fd_flag)
	int flags;
	
} process_fdnum_t;

//Process control block
typedef struct process_s
{
	//Spinlock protecting this process
	hal_spl_t spl;

	//State of the process
	process_state_t state;
	
	//Positive ID of the process
	int id;
	
	//ID of parent of this process
	int parent;
	
	//ID of the process group containing this process (one process group at a time may write to the console)
	int pgid;
	
	//Memory space of the process, kernel and CPU info
	mem_space_t *mem;
	
	//Number of threads executing in the process
	int nthreads;
	
	//Array of IDs of file descriptors present in the process
	process_fdnum_t *fd_array;
	int fd_count;
	
	//ID of file descriptor representing working directory
	int64_t fd_pwd;
	
	//Resource limits for the process
	struct rlimit rlimits[_RLIMIT_MAX];
	
	//Return value / exit status of the process, as passed on _exit
	int exitstatus;
	
	//Status for parent to wait on, tracked by kernel
	int waitstatus;
	
	//Location of user entry point in the process.
	uintptr_t entry;
	
	//Notification fired when a child changes status
	notify_src_t child_notify;
	
} process_t;

//Makes process table and sets up first process.
void process_init(void);

//Locks and returns the calling process's process control block.
process_t *process_lockcur(void);

//Looks up and locks the process with the given ID. Returns a pointer to it.
process_t *process_getlocked(id_t id);

//Looks up and locks a process entry that is unused. Returns a pointer to it.
process_t *process_locknew(void);

//Unlocks the given process
void process_unlock(process_t *pptr);

//Removes the calling thread from its process and sets the process to dead if none remain.
void process_leave(void);

//Adds the given file descriptor to the process, with a number at least "min".
//Overwrites an existing descriptor at "min" if overwrite is specified.
//If an ID was overwritten, it is returned in *old_id.
//Returns the number or a negative error number.
int process_addfd(id_t id, int min, bool overwrite, id_t *old_id);

//Returns the ID of the file descriptor with the given number in the calling process.
id_t process_getfdnum(int num);

//Clears the file descriptor number in the calling process.
//Returns the previous file descriptor ID that was referred there.
//Does not alter the reference-count of the file descriptor.
id_t process_clearfdnum(int num);

//Sets flags for the given file descriptor number in the calling process.
//Returns their new value.
int process_flagfdnum(int num, int set, int clr);

//Returns the ID of the file descriptor for the working directory of the calling process.
id_t process_getfdpwd(void);

//Copies a string to the calling process's memory.
//Returns 0 on success or a negative error number.
int process_strncpy_touser(void *dst_u, const void *src_k, size_t buflen);

//Waits for children of the calling process to have available status.
//Returns 0 on success or a negative error number. Places result in *out.
int process_wait(idtype_t id_type, int64_t id, int options, px_wait_t *out);

#endif //PROCESS_H
