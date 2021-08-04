//pipe.c
//Pipes in libc
//Bryan E. Topp <betopp@betopp.com> 2021

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <px.h>

int pipe2(int fildes[2], int flags)
{	
	//The kernel doesn't actually support anonymous pipes.
	//Make a tempfile FIFO and open it for read and for write, then unlink it.
	
	//Try as many times as we need to, to make a temp file with a unique name
	const char *name_templ = ".pipe_XXXXXXXX";
	char name_buf[16];
	fildes[0] = -1;
	fildes[1] = -1;
	while(1)
	{
		strncpy(name_buf, name_templ, sizeof(name_buf)-1);
		char *name = mktemp(name_buf);
		if(name == NULL)
			return -1; //mktemp sets errno
		
		fildes[0] = open(name, O_CREAT | O_EXCL | flags, S_IFIFO | 0600);
		if(fildes[0] == -1)
		{
			if(errno == EEXIST)
				continue; //Try again if that pipe name exists
			else
				return -1; //Other errors are actual failures, open sets errno
		}
		
		//Unlink the file underneath (unless somebody else came along and did it)
		funlinkat(AT_FDCWD, name, fildes[0], 0);
		break;
	}
	
	//Open a second file descriptor and set its close-on-exec flag appropriately
	fildes[1] = px_fd_find(fildes[0], "");
	if(fildes[1] < 0)
	{
		errno = -fildes[1];
		goto failure;
	}
	
	if(!(flags & O_CLOEXEC))
	{
		int keep_err = px_fd_flag(fildes[1], PX_FD_FLAG_KEEPEXEC, 0);
		if(keep_err < 0)
		{
			errno = -keep_err;
			goto failure;
		}
	}
	
	//Set one end of the pipe for reading and one for writing
	int rd_acc = px_fd_access(fildes[0], PX_FD_ACCESS_R, 0);
	if(rd_acc < 0)
	{
		errno = -rd_acc;
		goto failure;
	}

	int wr_acc = px_fd_access(fildes[1], PX_FD_ACCESS_W, 0);
	if(wr_acc < 0)
	{
		errno = -wr_acc;
		goto failure;
	}
	
	//Success
	return 0;

failure:
	
	if(fildes[0] >= 0)
	{
		px_fd_close(fildes[0]);
		fildes[0] = -1;
	}
	
	if(fildes[1] >= 0)
	{
		px_fd_close(fildes[1]);
		fildes[1] = -1;
	}
	
	return -1;
}

int pipe(int fildes[2])
{
	return pipe2(fildes, 0);
}
