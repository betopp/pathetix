//dir.c
//Directory descriptors in libc
//Bryan E. Topp <betopp@betopp.com> 2021

#include <dirent.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include <px.h>

struct _DIR_s
{
	int fd; //Underlying file descriptor
	char buf[512]; //Space 
};

DIR *fdopendir(int fd)
{
	DIR *retval = malloc(sizeof(DIR));
	if(retval == NULL)
	{
		errno = ENOMEM;
		return NULL;
	}
	
	memset(retval, 0, sizeof(*retval));
	retval->fd = fd;
	
	px_fd_flag(fd, 0, PX_FD_FLAG_KEEPEXEC); //clear keep-through-exec flag
	
	return retval;
}

int fdclosedir(DIR *dirp)
{
	free(dirp);
	return 0;
}

DIR *opendir(const char *filename)
{
	int fd = open(filename, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
	if(fd < 0)
		return NULL;
	
	DIR *retval = fdopendir(fd);
	if(retval == NULL)
	{
		close(fd);
		return NULL;
	}
	
	return retval;
}

struct dirent *readdir(DIR *dirp)
{
	px_fd_dirent_t d = {0};
	int read_result = px_fd_read(dirp->fd, &d, sizeof(d));
	if(read_result < 0)
	{
		errno = -read_result;
		return NULL;
	}
	if(read_result == 0)
	{
		return NULL;
	}

	memset(dirp->buf, 0, sizeof(dirp->buf));
	struct dirent *retval = (struct dirent*)(dirp->buf);
	
	retval->d_ino = d.ino;
	strncpy(retval->d_name, d.name, sizeof(dirp->buf) - sizeof(struct dirent) - 1);
	
	return retval;
}

int closedir(DIR *dirp)
{
	int fd = dirp->fd;
	fdclosedir(dirp);
	close(fd);
	return 0;
}
