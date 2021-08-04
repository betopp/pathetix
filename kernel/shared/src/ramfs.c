//ramfs.c
//In-memory root filesystem
//Bryan E. Topp <betopp@betopp.com> 2021

#include "ramfs.h"
#include "kassert.h"
#include "kspace.h"
#include "libcstubs.h"
#include "pipe.h"

#include <stdbool.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>

//Number of page-length allocations referred to in each level of RAMfs table
#define RAMFS_PAGENUM 500

//Indirect table of pointers
//Todo - can probably write this to be automatically recursive
//(like, always assume the last entry is to the root of one more indirection)
typedef struct ramfs_indir_s
{
	void *pages[RAMFS_PAGENUM];
} ramfs_indir_t;

//Information about each file in RAMfs
typedef struct ramfs_inode_s
{
	//Spinlock protecting this file
	hal_spl_t spl;
	
	//Number of references in filesystem
	int64_t refs_fs;
	
	//Number of references by file descriptors
	int64_t refs_fd;
	
	//File size in bytes
	off_t size;
	
	//File mode
	mode_t mode;
	
	//Device reference if mode is a character/block special
	uint64_t spec;
	
	//Direct data pages table (first RAMFS_PAGENUM pages)
	void *pages[RAMFS_PAGENUM];
	
	//Indirect table (following RAMFS_PAGENUM * RAMFS_PAGENUM pages)
	ramfs_indir_t *indir;
	
} ramfs_inode_t;

//Root directory inode - one "filesystem" reference to keep it around always. One "file descriptor" ref for init's PWD on startup.
static ramfs_inode_t ramfs_root = { .refs_fs = 1, .refs_fd = 1, .mode = S_IFDIR | 0777 };

//Interprets an inode number as an inode pointer.
//Inode numbers are just pointers shifted down, to make them positive, under the assumption that they're 2-byte-aligned
static ramfs_inode_t *ramfs_inode_ptr(ino_t ino)
{
	if(ino == 0)
	{
		//Special case - inode 0 is root.
		//Make sure root is set up. We don't lock because this is guaranteed to only happen while single-threaded.
		if(ramfs_root.pages[0] == NULL)
		{
			ramfs_root.pages[0] = kspace_alloc(hal_frame_size(), hal_frame_size());
			KASSERT(ramfs_root.pages[0] != NULL);
			static const px_fd_dirent_t root_contents[2] = {
				{ .name = ".", .ino = 0 },
				{ .name = "..", .ino = 0 },
			};
			KASSERT(sizeof(root_contents) < hal_frame_size());
			memcpy(ramfs_root.pages[0], root_contents, sizeof(root_contents));
			ramfs_root.size = sizeof(root_contents);
		}
		return &ramfs_root;
	}
	
	uintptr_t ino_ptr = ino;
	ino_ptr *= 2;
	return (ramfs_inode_t*)(ino_ptr);
}

//Interprets an inode pointer as an inode number.
static ino_t ramfs_inode_ino(ramfs_inode_t *ptr)
{
	KASSERT(sizeof(ino_t) >= sizeof(uintptr_t));
	
	if(ptr == &ramfs_root)
		return 0;
	
	uintptr_t ptr_int = (uintptr_t)ptr;
	KASSERT( (ptr_int % 2) == 0 );
	ptr_int /= 2;
	return (ino_t)ptr_int;
}

//Finds the page containing data for the given offset in the given inode.
//Optionally allocates that page if it does not exist.
//Outputs the page address in *ptr_out, or outputs NULL if it doesn't exist and won't be created.
//Returns 0 on success or a negative error number.
static int ramfs_getpage(ramfs_inode_t *iptr, off_t off, bool alloc, void **ptr_out)
{
	//Zero this initially, for error returns
	*ptr_out = NULL;
	
	//Negative offsets are always invalid
	if(off < 0)
		return -EINVAL;
	
	//Units of allocation are all memory pages
	size_t pagesize = hal_frame_size();
	off /= pagesize;
	
	//First RAMFS_PAGENUM pages are pointed to directly, by table in the inode
	if(off < RAMFS_PAGENUM)
	{
		if(iptr->pages[off] == NULL)
		{
			if(!alloc)
				return 0; //Not allocated and we don't want to
			
			iptr->pages[off] = kspace_alloc(pagesize, pagesize);
			if(iptr->pages[off] == NULL)
				return -ENOSPC; //Tried and failed to allocate data page
		}
		
		//Found it
		KASSERT(iptr->pages[off] != NULL);
		*ptr_out = iptr->pages[off];
		return 0;
	}
	
	//Next RAMFS_PAGENUM^2 pages go through an indirect table, pointing at tables
	off -= RAMFS_PAGENUM;
	if(off < RAMFS_PAGENUM * RAMFS_PAGENUM)
	{
		//Look up first table from pointer in inode
		if(iptr->indir == NULL)
		{
			if(!alloc)
				return 0; //No indirect table and we don't want one.
	
			//Need to allocate space for first table
			iptr->indir = kspace_alloc(sizeof(ramfs_indir_t), alignof(ramfs_indir_t));
			if(iptr->indir == NULL)
				return -ENOSPC; //No room for table
		}
		
		//Look up second table from entry in first table
		KASSERT(iptr->indir != NULL);
		if(iptr->indir->pages[off / RAMFS_PAGENUM] == NULL)
		{
			if(!alloc)
				return 0; //No second table and we don't want one.
			
			//Need to allocate space for second table
			iptr->indir->pages[off / RAMFS_PAGENUM] = kspace_alloc(sizeof(ramfs_indir_t), alignof(ramfs_indir_t));
			if(iptr->indir->pages[off / RAMFS_PAGENUM] == NULL)
				return -ENOSPC; //No room for second table
		}
		
		//Look up data page from entry in second table
		KASSERT(iptr->indir->pages[off / RAMFS_PAGENUM] != NULL);
		ramfs_indir_t *indir2 = iptr->indir->pages[off / RAMFS_PAGENUM];
		if(indir2->pages[off % RAMFS_PAGENUM] == NULL)
		{
			if(!alloc)
				return 0; //No data page and we don't want one.
		
			//Need to allocate space for data page
			indir2->pages[off % RAMFS_PAGENUM] = kspace_alloc(pagesize, pagesize);
			if(indir2->pages[off % RAMFS_PAGENUM] == NULL)
				return -ENOSPC; //No room for data page		
		}
		
		//Found it
		KASSERT(indir2->pages[off % RAMFS_PAGENUM] != NULL);
		*ptr_out = indir2->pages[off % RAMFS_PAGENUM];
		return 0;
	}
	
	//Could support double-indirect table but I don't care right now - 500*500*4096 gives us about 1GByte temp files.
	return -EFBIG;
}

//Reads data from the given inode into the given buffer.
static ssize_t ramfs_readat(ramfs_inode_t *iptr, off_t off, void *buf, ssize_t len)
{	
	//Have to read in chunks of at most one memory page
	size_t pagesize = hal_frame_size();
	
	//Read from each data page
	uint8_t *buf_remain = buf;
	ssize_t total_out = 0;
	while(1)
	{
		//Each time we can read, at most, one page.
		off_t chunksize = pagesize;
		
		//Cap to end-of-page
		off_t to_page = pagesize - (off % pagesize);
		if(chunksize > to_page)
			chunksize = to_page;
		
		//Cap to end-of-file
		off_t to_eof = iptr->size - off;
		if(chunksize > to_eof)
			chunksize = to_eof;
		
		//Cap to desired read length
		if(chunksize > len)
			chunksize = len;
		
		//Stop if there's nothing more to read
		if(chunksize <= 0)
			return total_out;
		
		//Find the page of data
		uint8_t *datapage_ptr = NULL;
		int datapage_err = ramfs_getpage(iptr, off, false, (void**)(&datapage_ptr));
		if(datapage_err < 0)
			return datapage_err;
		
		if(datapage_ptr != NULL)
		{
			//If the page of data exists, copy from the page, with appropriate offset		
			memcpy(buf_remain, datapage_ptr + (off % pagesize), chunksize);
		}
		else
		{
			//If there's no data page, then this is a hole in the file - read as zeroes.
			memset(buf_remain, 0, chunksize);
		}
		
		//Advance
		buf_remain += chunksize;
		total_out += chunksize;
		off += chunksize;
		len -= chunksize;
	}
}

//Writes data into the given inode.
static ssize_t ramfs_writeat(ramfs_inode_t *iptr, off_t off, const void *buf, ssize_t len)
{
	//Similar to read. Go page-by-page and copy data into the file.
	size_t pagesize = hal_frame_size();
	const uint8_t *buf_remain = buf;
	ssize_t total_out = 0;
	while(1)
	{
		off_t chunksize = pagesize;
		
		//Don't cap to EOF when writing
		off_t to_page = pagesize - (off % pagesize);
		if(chunksize > to_page)
			chunksize = to_page;
		
		if(chunksize > len)
			chunksize = len;
		
		if(chunksize <= 0)
			return total_out;
		
		//Find the page of data, and try to allocate if it doesn't exist.
		uint8_t *datapage_ptr = NULL;
		
		//Super temp - don't allow consuming all the machine's memory with RAMfs.
		//Needs to be configurable somehow...
		bool alloc = (hal_frame_count() * hal_frame_size()) > (32*1024*1024);
		int datapage_err = ramfs_getpage(iptr, off, alloc, (void**)(&datapage_ptr));
		if(datapage_err < 0)
			return datapage_err;
		
		if(datapage_ptr == NULL && !alloc)
			return -ENOSPC;
		
		//Page must exist if we're writing into it. The above code should fail if allocation fails.
		//Copy into the page, with appropriate offset
		KASSERT(datapage_ptr != NULL);
		memcpy(datapage_ptr + (off % pagesize), buf_remain, chunksize);
		
		//Advance
		buf_remain += chunksize;
		total_out += chunksize;
		off += chunksize;
		len -= chunksize;
		
		//If we've written past EOF, expand the size of the file.
		if(off > iptr->size)
			iptr->size = off;
	}	
}

//Truncates the given already-locked inode, freeing unused data pages.
static void ramfs_trunc_inode(ramfs_inode_t *iptr, off_t size)
{
	//Change the size of the file.
	//This permits reading zeroes past the old end of the file, if expanding.
	iptr->size = size;
	
	//Free any pages allocated beyond the size.
	//This zeroes data past the new end of the file, if contracting.
	size_t pagesize = hal_frame_size();
	off_t killpage = (iptr->size + pagesize - 1) / pagesize; //First page that is no longer needed
	
	//Free direct data pages
	for(int pp = killpage; pp < RAMFS_PAGENUM; pp++)
	{
		if(iptr->pages[pp] != NULL)
		{
			kspace_free(iptr->pages[pp], pagesize);
			iptr->pages[pp] = NULL;
		}
	}

	//Free indirect data pages and tables
	if(iptr->indir != NULL)
	{
		//Indirect system happens after first RAMFS_PAGENUM pages
		killpage -= RAMFS_PAGENUM;
		if(killpage < 0)
			killpage = 0;
		
		//Free indirect data pages
		for(int pp = killpage; pp < RAMFS_PAGENUM * RAMFS_PAGENUM; pp++)
		{
			ramfs_indir_t *indir2 = iptr->indir->pages[pp / RAMFS_PAGENUM];
			if(indir2 == NULL)
			{
				//Whole table doesn't exist - skip trying to free the following RAMFS_PAGENUM - 1 entries in it.
				pp += RAMFS_PAGENUM - 1;
				continue;
			}
			
			if(indir2->pages[pp % RAMFS_PAGENUM] != NULL)
			{
				kspace_free(indir2->pages[pp % RAMFS_PAGENUM], pagesize);
				indir2->pages[pp % RAMFS_PAGENUM] = NULL;
			}
		}
		
		//Free any empty second-level indirect tables
		for(int pp = 0; pp < RAMFS_PAGENUM; pp++)
		{
			ramfs_indir_t *indir2 = iptr->indir->pages[pp];
			if(indir2 == NULL)
				continue;
			
			bool any_used = false;
			for(int ee = 0; ee < RAMFS_PAGENUM; ee++)
			{
				if(indir2->pages[ee] != NULL)
				{
					any_used = true;
					break;
				}
			}
			
			if(!any_used)
			{
				kspace_free(iptr->indir->pages[pp], sizeof(ramfs_indir_t));
				iptr->indir->pages[pp] = NULL;
			}
		}
	}	
}

//Deletes the given inode, assuming it has no references.
static void ramfs_delete(ramfs_inode_t *iptr)
{
	//Should be locked with no references at time of deletion
	KASSERT(iptr->spl > 0);
	KASSERT(iptr->refs_fd == 0);
	KASSERT(iptr->refs_fs == 0);
	
	//Truncate to nothing to free all data pages
	ramfs_trunc_inode(iptr, 0);
	
	//Free the indirect table if any - should already be emptied by the truncation.
	if(iptr->indir != NULL)
	{
		for(int ii = 0; ii < RAMFS_PAGENUM; ii++)
		{
			KASSERT(iptr->indir->pages[ii] == NULL);
		}
		
		kspace_free(iptr->indir, sizeof(ramfs_indir_t));
		iptr->indir = NULL;
	}
	
	//Direct data pages should all be gone
	for(int ii = 0; ii < RAMFS_PAGENUM; ii++)
	{
		KASSERT(iptr->pages[ii] == NULL);
	}
	
	//Remove reference to pipe if any
	if(S_ISFIFO(iptr->mode))
	{
		pipe_decr(iptr->spec, 0);
	}
	
	//Free the inode itself
	kspace_free(iptr, sizeof(*iptr));
}


//Makes and returns a new file descriptor for the given inode, already locked.
//The new file descriptor is returned with one reference, still locked.
static fd_t *ramfs_newfd(ramfs_inode_t *iptr)
{
	KASSERT(iptr->spl > 0);
	
	fd_t *fptr = fd_new();
	if(fptr == NULL)
	{
		//No room for file descriptor
		return NULL;
	}
	
	fptr->off = 0;
	fptr->refs = 1;
	fptr->ino = ramfs_inode_ino(iptr);
	fptr->mode = iptr->mode;
	fptr->spec = iptr->spec;
	
	iptr->refs_fd++;
	KASSERT(iptr->refs_fd > 0);
	

	return fptr;
}

//Makes and returns a new file descriptor for the given existing inode number.
//Will lock the inode if not the same as the given "at" inode, already locked.
//Unlocks the file descriptor and returns its ID.
static id_t ramfs_newfd_existing(ramfs_inode_t *at, ino_t ino)
{
	//Lock the target inode if necessary
	ramfs_inode_t *iptr;
	if(ramfs_inode_ino(at) != ino)
	{
		//Don't have the target inode locked already
		iptr = ramfs_inode_ptr(ino);
		hal_spl_lock(&(iptr->spl));
	}
	else
	{
		//Already have the inode locked
		iptr = at;
	}
	
	//Try to make a file descriptor for it
	fd_t *fptr = ramfs_newfd(iptr);
	
	//Unlock the target inode regardless of the result
	if(ramfs_inode_ino(at) != ino)
	{
		hal_spl_unlock(&(iptr->spl));
	}
	
	//See if we have a file descriptor to return
	if(fptr == NULL)
		return -ENFILE;
	
	KASSERT(fptr->refs == 1);
	id_t retval = fptr->id;
	fd_unlock(fptr);
	return retval;
}

id_t ramfs_create(fd_t *fd, const char *name, mode_t mode, uint64_t spec)
{	
	ramfs_inode_t *iptr = ramfs_inode_ptr(fd->ino);
	hal_spl_lock(&(iptr->spl));
	
	if(!S_ISDIR(iptr->mode))
	{
		hal_spl_unlock(&(iptr->spl));
		return -ENOTDIR;
	}
	
	//Read through the directory to make sure the name doesn't already exist
	off_t off = 0;
	px_fd_dirent_t buf = {0};
	while(off < iptr->size)
	{
		ssize_t read_err = ramfs_readat(iptr, off, &buf, sizeof(buf));
		if(read_err < 0)
		{
			hal_spl_unlock(&(iptr->spl));
			return read_err;
		}
		KASSERT(read_err == sizeof(buf));
		
		if(!strncmp(buf.name, name, sizeof(buf.name)))
		{
			hal_spl_unlock(&(iptr->spl));
			return -EEXIST;
		}

		off += sizeof(buf);
	}
	
	//Alright, the name doesn't already exist in the directory. We can make a file.
	
	//Make a new inode for it.
	ramfs_inode_t *newinode = kspace_alloc(sizeof(ramfs_inode_t), alignof(ramfs_inode_t));
	if(newinode == NULL)
	{
		//Ran out of memory
		hal_spl_unlock(&(iptr->spl));
		return -ENOSPC;
	}
	hal_spl_lock(&(newinode->spl));
	newinode->mode = mode;
	newinode->spec = spec;
	
	//If we're making a directory, make sure it starts with "." and ".." entries.
	if(S_ISDIR(mode))
	{
		const px_fd_dirent_t initial_entries[2] =
		{
			{ .name = ".", .ino = ramfs_inode_ino(newinode) },
			{ .name = "..", .ino = ramfs_inode_ino(iptr) },
		};
		ssize_t written = ramfs_writeat(newinode, 0, initial_entries, sizeof(initial_entries));
		if(written != (ssize_t)sizeof(initial_entries))
		{
			//Failed to write initial directory entries
			hal_spl_unlock(&(iptr->spl));
			ramfs_delete(newinode);
			KASSERT(written < 0);
			return written;
		}
	}
	
	//Make a file descriptor that references the new inode.
	fd_t *newfd = ramfs_newfd(newinode);
	if(newfd == NULL)
	{
		//No room for file descriptor
		hal_spl_unlock(&(iptr->spl));
		ramfs_delete(newinode);
		return -ENFILE;
	}
	
	//Append a directory entry in the directory, referencing the new inode
	memset(&buf, 0, sizeof(buf));
	strncpy(buf.name, name, sizeof(buf.name)-1);
	buf.ino = ramfs_inode_ino(newinode);
	buf.next = off + sizeof(buf);
	
	ssize_t dirent_result = ramfs_writeat(iptr, off, &buf, sizeof(buf));
	if(dirent_result < 0)
	{
		//Error appending directory entry
		
		newfd->refs = 0;
		fd_unlock(newfd);
		
		ramfs_delete(newinode);
		
		iptr->size = off; //Truncate
		hal_spl_unlock(&(iptr->spl));
		
		return dirent_result;
	}
	KASSERT(dirent_result == sizeof(buf));
	newinode->refs_fs++;
	
	//Store reference to the pipe, if any
	if(S_ISFIFO(newinode->mode))
	{
		pipe_incr(newinode->spec, 0);
	}
	
	//Success
	hal_spl_unlock(&(newinode->spl));
	hal_spl_unlock(&(iptr->spl));
	id_t retval = newfd->id;
	fd_unlock(newfd);
	return retval;
}

id_t ramfs_find(fd_t *fd, const char *name)
{
	ramfs_inode_t *iptr = ramfs_inode_ptr(fd->ino);
	hal_spl_lock(&(iptr->spl));
	
	//Trivial case - looking up "/". Always return root.
	if(name[0] == '/' && name[1] == '\0')
	{
		id_t retval = ramfs_newfd_existing(iptr, 0);
		hal_spl_unlock(&(iptr->spl));
		return retval;
	}
	
	//Trivial case - looking up "". Always return the given file.
	if(name[0] == '\0')
	{
		id_t retval = ramfs_newfd_existing(iptr, fd->ino);
		hal_spl_unlock(&(iptr->spl));
		return retval;
	}
	
	//All other cases must be relative to a directory.
	if(!S_ISDIR(iptr->mode))
	{
		hal_spl_unlock(&(iptr->spl));
		return -ENOTDIR;
	}
	
	//Directories are an array of fixed-length directory entries.
	//Read and examine each one.
	off_t off = 0;
	px_fd_dirent_t buf = {0};
	while(off < iptr->size)
	{
		ssize_t read_err = ramfs_readat(iptr, off, &buf, sizeof(buf));
		if(read_err < 0)
		{
			hal_spl_unlock(&(iptr->spl));
			return read_err;
		}
		KASSERT(read_err == sizeof(buf));
		
		if(!strncmp(buf.name, name, sizeof(buf.name)))
		{
			//Found a directory entry with the name we want.
			id_t retval = ramfs_newfd_existing(iptr, buf.ino);
			hal_spl_unlock(&(iptr->spl));
			return retval;
		}

		off += sizeof(buf);
	}
	
	//Not found
	hal_spl_unlock(&(iptr->spl));
	return -ENOENT;
}

ssize_t ramfs_read(fd_t *fd, void *buf, size_t len)
{
	if(len >= SSIZE_MAX)
		len = SSIZE_MAX;
	
	ramfs_inode_t *iptr = ramfs_inode_ptr(fd->ino);
	hal_spl_lock(&(iptr->spl));
	
	if(S_ISDIR(iptr->mode))
	{
		//Reading directory. Normalize offset of file descriptor to an integer number of directory entries.
		if(fd->off % sizeof(px_fd_dirent_t) != 0)
			fd->off -= fd->off % sizeof(px_fd_dirent_t);
		
		//Normalize length to an integer number of directory entries.
		if(len % sizeof(px_fd_dirent_t) != 0)
			len -= len % sizeof(px_fd_dirent_t);
		
		//We store directory entries in the same format as they are returned.
		//Just copy them out.
		ssize_t retval = ramfs_readat(iptr, fd->off, buf, (ssize_t)len);
		hal_spl_unlock(&(iptr->spl));
		
		if(retval > 0)
			fd->off += retval;
		
		return retval;
	}
	else
	{
		//Reading normal file
		ssize_t retval = ramfs_readat(iptr, fd->off, buf, (ssize_t)len);
		hal_spl_unlock(&(iptr->spl));
		
		if(retval > 0)
			fd->off += retval;
		
		return retval;
	}
}

ssize_t ramfs_write(fd_t *fd, const void *buf, size_t len)
{
	if(len >= SSIZE_MAX)
		len = SSIZE_MAX;
	
	ramfs_inode_t *iptr = ramfs_inode_ptr(fd->ino);
	hal_spl_lock(&(iptr->spl));
	
	
	ssize_t retval = ramfs_writeat(iptr, fd->off, buf, (ssize_t)len);
	
	hal_spl_unlock(&(iptr->spl));
	
	if(retval > 0)
		fd->off += retval;
	
	return retval;	
}

ssize_t ramfs_stat(fd_t *fd, px_fd_stat_t *buf, size_t len)
{
	ramfs_inode_t *iptr = ramfs_inode_ptr(fd->ino);
	hal_spl_lock(&(iptr->spl));
	
	px_fd_stat_t st = {0};
	st.ino = fd->ino;
	st.size = iptr->size;
	st.mode = iptr->mode;
	st.spec = iptr->spec;
	
	hal_spl_unlock(&(iptr->spl));
	
	size_t len_copy = (len < sizeof(px_fd_stat_t)) ? len : sizeof(px_fd_stat_t);
	memcpy(buf, &st, len_copy);
	return len_copy;
}

int ramfs_trunc(fd_t *fd, off_t size)
{
	if(size < 0)
		return -EINVAL;
	
	if(size >= RAMFS_PAGENUM * RAMFS_PAGENUM * (off_t)hal_frame_size())
		return -EFBIG;
	
	ramfs_inode_t *iptr = ramfs_inode_ptr(fd->ino);
	hal_spl_lock(&(iptr->spl));
	
	if(S_ISDIR(iptr->mode))
	{
		hal_spl_unlock(&(iptr->spl));
		return -EISDIR;
	}
	
	ramfs_trunc_inode(iptr, size);
	
	hal_spl_unlock(&(iptr->spl));
	return 0;
}

int ramfs_unlink(fd_t *fd, const char *name, ino_t only_ino, int rmdir)
{
	//Don't permit unlinking "." and "..".
	if( (strcmp(name, ".") == 0) || (strcmp(name, "..") == 0) )
		return -EINVAL;
	
	//Make sure we're operating on a directory
	ramfs_inode_t *iptr = ramfs_inode_ptr(fd->ino);
	hal_spl_lock(&(iptr->spl));
	
	if(!S_ISDIR(iptr->mode))
	{
		hal_spl_unlock(&(iptr->spl));
		return -ENOTDIR;
	}
	
	//Directories are an array of fixed-length directory entries.
	//Read and examine each one.
	off_t off = 0;
	px_fd_dirent_t buf = {0};
	while(off < iptr->size)
	{
		//Read next entry
		ssize_t read_err = ramfs_readat(iptr, off, &buf, sizeof(buf));
		if(read_err < 0)
		{
			hal_spl_unlock(&(iptr->spl));
			return read_err;
		}
		KASSERT(read_err == sizeof(buf));
		
		if(strncmp(buf.name, name, sizeof(buf.name)) != 0)
		{
			//Wrong name, keep looking
			off += sizeof(buf);
			continue;
		}
		
		//Found the directory entry with the name we're trying to unlink.
		
		//If we asked to unlink a specific name+inode combo, make sure this is it.
		if( (only_ino != 0) && (buf.ino != only_ino) )
		{
			hal_spl_unlock(&(iptr->spl));
			return -EDEADLK; //See BSD funlinkat()
		}
		
		//Enforce rules about removing directories.
		ramfs_inode_t *found = ramfs_inode_ptr(buf.ino);
		hal_spl_lock(&(found->spl));
		if(S_ISDIR(found->mode))
		{
			if(!rmdir)
			{
				//Not asked to remove a directory, but that's what this is
				hal_spl_unlock(&(found->spl));
				hal_spl_unlock(&(iptr->spl));
				return -EISDIR;
			}
			
			if(found->size != 2 * sizeof(px_fd_dirent_t))
			{
				//More than just "." and ".." entries - not empty.
				hal_spl_unlock(&(found->spl));
				hal_spl_unlock(&(iptr->spl));
				return -ENOTEMPTY;
			}
		}
		else
		{
			if(rmdir)
			{
				//Asked to remove a directory, but it's not
				hal_spl_unlock(&(found->spl));
				hal_spl_unlock(&(iptr->spl));
				return -ENOTDIR;
			}
		}
		
		//Remove the directory entry.
		//Overwrite it with the last entry in this directory.
		//Then, truncate away the last entry.
		//Be very careful of failures that occur here, which could leave the directory corrupt.
		
		KASSERT(iptr->size % sizeof(px_fd_dirent_t) == 0);
		px_fd_dirent_t moved = {0};
		ssize_t moved_read_err = ramfs_readat(iptr, iptr->size - sizeof(px_fd_dirent_t), &moved, sizeof(moved));
		if(moved_read_err < 0)
		{
			hal_spl_unlock(&(found->spl));
			hal_spl_unlock(&(iptr->spl));
			return moved_read_err;
		}
		KASSERT(moved_read_err == sizeof(moved));
	
		ssize_t moved_write_err = ramfs_writeat(iptr, off, &moved, sizeof(moved));
		if(moved_write_err < 0)
		{
			hal_spl_unlock(&(found->spl));
			hal_spl_unlock(&(iptr->spl));
			return moved_write_err;
		}
		KASSERT(moved_write_err == sizeof(moved));
		ramfs_trunc_inode(iptr, iptr->size - sizeof(px_fd_dirent_t));
		
		//The file we're unlinking now has one less reference.
		//If nobody has it open, delete it. Otherwise, we just decrement the ref count and move on.
		found->refs_fs--;
		if(found->refs_fs <= 0 && found->refs_fd <= 0)
		{
			ramfs_delete(found);
			found = NULL;
		}
		else
		{
			hal_spl_unlock(&(found->spl));
			found = NULL;
		}
		
		//Successfully unlinked the file
		hal_spl_unlock(&(iptr->spl));
		return 0;
	}
	
	//Didn't find the name to unlink.
	hal_spl_unlock(&(iptr->spl));
	return -ENOENT;
}

void ramfs_close(fd_t *fd)
{
	ramfs_inode_t *iptr = ramfs_inode_ptr(fd->ino);
	hal_spl_lock(&(iptr->spl));
	
	//With this file descriptor closing, there's one less reference to the inode.
	iptr->refs_fd--;
	KASSERT(iptr->refs_fd >= 0);
	
	//If the inode now has no references, get rid of it
	if(iptr->refs_fd <= 0 && iptr->refs_fs <= 0)
	{
		ramfs_delete(iptr);
		return;
	}
	
	hal_spl_unlock(&(iptr->spl));
	return;
}

int ramfs_access(fd_t *fd, int set, int clr)
{
	//Todo - check permissions
	//Change access accordingly
	fd->access |= set;
	fd->access &= ~clr;
	return fd->access;
}
