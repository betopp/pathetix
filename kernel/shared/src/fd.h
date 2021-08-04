//fd.h
//File descriptors
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef FD_H
#define FD_H

#include <sys/types.h>
#include "hal_spl.h"

#include "px.h"

//State of file descriptor
typedef enum fd_state_e
{
	FD_STATE_NONE = 0, //No file descriptor in this entry
	FD_STATE_READY, //File descriptor exists and can be used for operation
	FD_STATE_WAIT, //File descriptor is in the middle of an operation - a thread is waiting for it to finish.
	FD_STATE_MAX
} fd_state_t;

//File descriptor
typedef struct fd_s
{
	//Spinlock protecting this file descriptor
	hal_spl_t spl;
	
	//State of file descriptor
	fd_state_t state;
	
	//ID of the file descriptor, as referred to in processes
	id_t id;
	
	//Reference-count - how many times this file descriptor is referred to
	int64_t refs;
	
	//Identifier of file on root RAMfs
	ino_t ino;
	
	//File pointer - next byte read/written
	off_t off;
	
	//Mode of file at the time it was opened.
	mode_t mode;
	
	//Special number of the file at the time it was opened.
	uint64_t spec;
	
	//Access flags enabled on the file descriptor
	int access;
	
	//Threads to wake up when the file descriptor becomes ready
	id_t *waketid_array;
	int waketid_count;
	
} fd_t;

//Sets up file descriptors
void fd_init(void);

//Makes a new file descriptor. Returns a pointer to it, still locked.
//Will be returned with 0 refs! Don't unlock before making a ref.
fd_t *fd_new(void);

//Unlocks the given file descriptor. Deletes it if there's no refs.
void fd_unlock(fd_t *fd);

//Looks up and locks the file descriptor with the given ID. Returns a pointer to it.
fd_t *fd_getlocked(id_t id);

//Makes a new file in the given directory with the given name. Returns a descriptor for it.
//The descriptor starts with one reference.
id_t fd_create(id_t id, const char *name, mode_t mode, uint64_t spec);

//Searches the given open directory and, if the file is found, makes a new file descriptor for it.
//The descriptor starts with one reference.
id_t fd_find(id_t at, const char *name);

//Performs a seek on the given file descriptor by ID.
off_t fd_seek(id_t id, off_t off, int whence);

//Performs a read from the given file descriptor by ID.
ssize_t fd_read(id_t id, void *buf, size_t len);

//Performs a write to the given file descriptor by ID.
ssize_t fd_write(id_t id, const void *buf, size_t len);

//Retrieves status information about the given file descriptor by ID.
ssize_t fd_stat(id_t id, px_fd_stat_t *buf, size_t len);

//Changes the size of a file referenced by the given file descriptor by ID.
int fd_trunc(id_t id, off_t size);

//Removes a directory entry. Optionally restricts the operation to only removing a link to a particular file.
int fd_unlink(id_t dirfd, const char *name, id_t reffd, int rmdir);

//Performs device-specific or RPC-oriented operations on a file descriptor by ID.
int fd_ioctl(id_t id, uint64_t request, void *ptr, size_t len);

//Changes access mode of a file descriptor by ID.
int fd_access(id_t id, int set, int clr);

//Increments the reference-count of the given file descriptor.
//Returns the number of references on success or a negative error number.
int64_t fd_incr(id_t id);

//Decrements the reference-count of the given file descriptor and deletes it if zero.
//Returns the number of references on success or a negative error number.
int64_t fd_decr(id_t id);


//Blocks during a filesystem operation.
//Sets the file descriptor to "wait" and releases the lock on the file descriptor.
//Attempts to re-grab the lock when the thread is woken up.
fd_t *fd_block(fd_t *fd);

#endif //FD_H
