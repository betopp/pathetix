//fd.c
//File descriptors
//Bryan E. Topp <betopp@betopp.com> 2021

#include "fd.h"
#include "kspace.h"
#include "kassert.h"
#include "ramfs.h"
#include "devs.h"
#include "con.h"
#include "pipe.h"

#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

//File descriptor table
static fd_t *fd_array;
static int fd_count;

//Functions supported by device drivers
typedef struct fd_devfuncs_s
{
	ssize_t (*read) (int minor, void *buf, size_t len);
	ssize_t (*write)(int minor, const void *buf, size_t len);
	int     (*ioctl)(int minor, uint64_t request, void *ptr, size_t len);
} fd_devfuncs_t;

//Device switch - all kernel devices are character devices
#define DEV_MAX 16
static const fd_devfuncs_t fd_devfuncs[DEV_MAX] = 
{
	[0] = {
		.read = dev_null_read,
		.write = dev_null_write,
	},
	[1] = {
		.read = con_read,
		.write = con_write,
		.ioctl = con_ioctl,
	},
};

void fd_init(void)
{
	int fds = 4096; //Todo - configurable
	fd_array = kspace_alloc(sizeof(fd_t) * fds, alignof(fd_t));
	KASSERT(fd_array != NULL);
	fd_count = fds;
}

fd_t *fd_new(void)
{
	for(int ff = 0; ff < fd_count; ff++)
	{
		fd_t *fptr = &(fd_array[ff]);
		if(hal_spl_try(&(fptr->spl)))
		{
			if(fptr->state == FD_STATE_NONE)
			{
				//Found a spot for the file descriptor.
				//Give it an ID that indicates the position in the table (modulo the size of the table).
				//Advance through IDs as we reuse table entries.
				if(fptr->id == 0)
					fptr->id = ff;
				
				fptr->id += fd_count;
				
				KASSERT( (fptr->id % fd_count) == ff );
				
				fptr->state = FD_STATE_READY;
				return fptr; //Still locked
			}
			hal_spl_unlock(&(fptr->spl));
		}
	}
	
	//No room
	return NULL;
}

void fd_unlock(fd_t *fd)
{
	KASSERT(fd->state != FD_STATE_NONE);
	KASSERT(fd->refs >= 0);
	if(fd->refs == 0)
	{
		//Decrement pipe reference-counts, if this referenced a pipe
		if(S_ISFIFO(fd->mode))
		{
			if(fd->access & PX_FD_ACCESS_R)
				pipe_decr(fd->spec, PX_FD_ACCESS_R);
			
			if(fd->access & PX_FD_ACCESS_W)
				pipe_decr(fd->spec, PX_FD_ACCESS_W);
		}
		
		//Decrement inode reference counts
		ramfs_close(fd);
		
		//Last reference to this file descriptor closed and structure released.
		KASSERT(fd->state == FD_STATE_READY);
		fd->state = FD_STATE_NONE;
		fd->ino = 0;
		fd->off = 0;
		fd->spec = 0;
		fd->access = 0;
	}
	
	hal_spl_unlock(&(fd->spl));
}

fd_t *fd_getlocked(id_t id)
{
	if(id < 0)
	{
		return NULL;
	}
	
	fd_t *fdptr = &(fd_array[id % fd_count]);
	hal_spl_lock(&(fdptr->spl));
	
	if(fdptr->id != id)
	{
		//Empty/reused array entry
		hal_spl_unlock(&(fdptr->spl));
		return NULL;
	}
	
	//Success
	return fdptr;
}

id_t fd_create(id_t at, const char *name, mode_t mode, uint64_t spec)
{
	fd_t *at_fptr = fd_getlocked(at);
	if(at_fptr == NULL)
		return -EBADF;
	
	//Todo - when we support "RPC files" or similar, allow those to take this request
	if(!S_ISDIR(at_fptr->mode))
	{
		fd_unlock(at_fptr);
		return -ENOTDIR;
	}
	
	//If we're making a pipe file, make a new pipe number every time.
	if(S_ISFIFO(mode))
	{
		int64_t pipeid = pipe_new(); //Starts with one reference
		if(pipeid < 0)
		{
			fd_unlock(at_fptr);
			return pipeid;
		}
		spec = pipeid;
	}

	id_t retval = ramfs_create(at_fptr, name, mode, spec);
	fd_unlock(at_fptr);
	
	//Release the first reference to the pipe - the RAMfs should keep a reference to it on success
	if(S_ISFIFO(mode))
		pipe_decr(spec, 0);
	
	return retval;
}

id_t fd_find(id_t at, const char *name)
{
	fd_t *at_fptr = fd_getlocked(at);
	if(at_fptr == NULL)
		return -EBADF;
	
	//Note - don't check for is-directory here.
	//We permit finding "" on anything to open another file descriptor to it.

	id_t retval = ramfs_find(at_fptr, name);
	fd_unlock(at_fptr);
	return retval;
}

off_t fd_seek(id_t id, off_t off, int whence)
{
	fd_t *fptr = fd_getlocked(id);
	if(fptr == NULL)
		return -EBADF;
	
	if(S_ISCHR(fptr->mode) || S_ISFIFO(fptr->mode))
	{
		fd_unlock(fptr);
		return -ESPIPE;
	}
	
	px_fd_stat_t st = {0};
	ssize_t stat_err = ramfs_stat(fptr, &st, sizeof(st));
	if(stat_err < 0)
	{
		fd_unlock(fptr);
		return stat_err;
	}
	
	switch(whence)
	{
		case SEEK_SET:
			fptr->off = off;
			break;
		case SEEK_CUR:
			fptr->off += off;
			break;
		case SEEK_END:
			fptr->off = st.size + off;
			break;
		default:
			fd_unlock(fptr);
			return -EINVAL;
	}
	
	if(fptr->off < 0)
		fptr->off = 0;
	
	off_t retval = fptr->off;
	fd_unlock(fptr);
	return retval;
	
}

ssize_t fd_read(id_t id, void *buf, size_t len)
{
	fd_t *fptr = fd_getlocked(id);
	if(fptr == NULL)
		return -EBADF;
	
	if(S_ISCHR(fptr->mode))
	{
		int major = (fptr->spec >> 16) & 0xFFFF;
		int minor = (fptr->spec) & 0xFFFF;
		fd_unlock(fptr);
		
		if(major < 0 || major >= DEV_MAX)
			return -ENXIO;
		
		if(fd_devfuncs[major].read == NULL)
			return -ENOTTY;
		
		return (*fd_devfuncs[major].read)(minor, buf, len);
	}
	
	if(S_ISFIFO(fptr->mode))
	{
		int64_t pipe = fptr->spec;
		fd_unlock(fptr);
		
		return pipe_read(pipe, buf, len);
	}
	
	ssize_t retval = ramfs_read(fptr, buf, len);
	fd_unlock(fptr);
	return retval;	
}

ssize_t fd_write(id_t id, const void *buf, size_t len)
{
	fd_t *fptr = fd_getlocked(id);
	if(fptr == NULL)
		return -EBADF;
	
	if(S_ISCHR(fptr->mode))
	{
		int major = (fptr->spec >> 16) & 0xFFFF;
		int minor = (fptr->spec) & 0xFFFF;
		fd_unlock(fptr);
		
		if(major < 0 || major >= DEV_MAX)
			return -ENXIO;
		
		if(fd_devfuncs[major].write == NULL)
			return -ENOTTY;
		
		return (*fd_devfuncs[major].write)(minor, buf, len);
	}
	
	if(S_ISFIFO(fptr->mode))
	{
		int64_t pipe = fptr->spec;
		fd_unlock(fptr);
		
		return pipe_write(pipe, buf, len);
	}
	
	ssize_t retval = ramfs_write(fptr, buf, len);
	fd_unlock(fptr);
	return retval;	
}

ssize_t fd_stat(id_t id, px_fd_stat_t *buf, size_t len)
{
	fd_t *fptr = fd_getlocked(id);
	if(fptr == NULL)
		return -EBADF;
	
	ssize_t retval = ramfs_stat(fptr, buf, len);
	fd_unlock(fptr);
	return retval;	
}

int fd_trunc(id_t id, off_t size)
{
	fd_t *fptr = fd_getlocked(id);
	if(fptr == NULL)
		return -EBADF;
	
	if(S_ISCHR(fptr->mode))
		return -ENOTTY;
	
	if(S_ISDIR(fptr->mode))
		return -EISDIR;
	
	int retval = ramfs_trunc(fptr, size);
	fd_unlock(fptr);
	return retval;
}

int fd_unlink(id_t dirfd, const char *name, id_t reffd, int rmdir)
{	
	//If we have a specific file that we want to unlink, find its inode number
	ino_t refino = 0;
	if(reffd != 0)
	{
		fd_t *reffdptr = fd_getlocked(reffd);
		if(reffdptr == NULL)
			return -EBADF;
		
		refino = reffdptr->ino;
		
		fd_unlock(reffdptr);
	}
	
	fd_t *fptr = fd_getlocked(dirfd);
	if(fptr == NULL)
		return -EBADF;
	
	if(!S_ISDIR(fptr->mode))
	{
		fd_unlock(fptr);
		return -ENOTDIR;
	}
	
	int retval = ramfs_unlink(fptr, name, refino, rmdir);
	fd_unlock(fptr);
	return retval;
}

int fd_ioctl(id_t id, uint64_t request, void *ptr, size_t len)
{
	fd_t *fptr = fd_getlocked(id);
	if(fptr == NULL)
		return -EBADF;
	
	if(S_ISCHR(fptr->mode))
	{
		int major = (fptr->spec >> 16) & 0xFFFF;
		int minor = (fptr->spec) & 0xFFFF;
		fd_unlock(fptr);
		
		if(major < 0 || major >= DEV_MAX)
			return -ENXIO;
		
		if(fd_devfuncs[major].ioctl == NULL)
			return -ENOTTY;
		
		return (*fd_devfuncs[major].ioctl)(minor, request, ptr, len);
	}
	
	fd_unlock(fptr);
	return -ENOTTY;	
}

int fd_access(id_t id, int set, int clr)
{
	//Disallow trying to change spare bits
	if( (set | clr) & ~(PX_FD_ACCESS_R | PX_FD_ACCESS_W | PX_FD_ACCESS_X) )
		return -EINVAL;
	
	fd_t *fptr = fd_getlocked(id);
	if(fptr == NULL)
		return -EBADF;
	
	int oldval = fptr->access;	
	int newval = ramfs_access(fptr, set, clr);
	
	if(S_ISFIFO(fptr->mode) && (newval >= 0))
	{
		if(newval & (~oldval) & PX_FD_ACCESS_R)
			pipe_incr(fptr->spec, PX_FD_ACCESS_R);
		if(newval & (~oldval) & PX_FD_ACCESS_W)
			pipe_incr(fptr->spec, PX_FD_ACCESS_W);
		
		if(oldval & (~newval) & PX_FD_ACCESS_R)
			pipe_decr(fptr->spec, PX_FD_ACCESS_R);
		if(oldval & (~newval) & PX_FD_ACCESS_W)
			pipe_decr(fptr->spec, PX_FD_ACCESS_W);
	}
	
	fd_unlock(fptr);
	return newval;
}

int64_t fd_incr(id_t id)
{
	fd_t *fptr = fd_getlocked(id);
	if(fptr == NULL)
		return -EBADF;
	
	fptr->refs++;
	KASSERT(fptr->refs > 0);
	
	int64_t retval = fptr->refs;
	fd_unlock(fptr);
	return retval;
}

int64_t fd_decr(id_t id)
{
	fd_t *fptr = fd_getlocked(id);
	if(fptr == NULL)
		return -EBADF;
	
	fptr->refs--;
	KASSERT(fptr->refs >= 0);

	int64_t retval = fptr->refs;
	fd_unlock(fptr); //Handles refs=0 case, deletes FD
	return retval;
}
