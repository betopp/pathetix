//getcwd.c
//Really horrible implementation of getcwd
//Bryan E. Topp <betopp@betopp.com> 2021

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <px.h>

//Is there a "right" way to do this? This seems horrible.
//I guess I could keep buffers in the kernel with the whole path used to find any given file.

//Given a directory file descriptor, searches its parent directory to find its name.
//Puts the name in the buffer, backwards. Returns how much space was used.
ssize_t _getcwd_name(int fd, char *buf, size_t size)
{
	//Stat the open file to find its ino, so we can match that with a dirent in the parent
	px_fd_stat_t st = {0};
	int staterr = px_fd_stat(fd, &st, sizeof(st));
	if(staterr < 0)
		return staterr;
	
	if(st.ino == 0) //Root dir
	{
		if(size > 0)
		{
			*buf = '/';
			return 1;
		}
	}
	
	int parent = px_fd_find(fd, "..");
	if(parent < 0)
		return parent;
	
	//Read all dirents from the parent
	while(1)
	{
		px_fd_dirent_t dirent = {0};
		ssize_t dirent_result = px_fd_read(parent, &dirent, sizeof(dirent));
		if(dirent_result < 0)
		{
			px_fd_close(parent);
			return dirent_result;
		}
		
		if(dirent_result == 0)
		{
			px_fd_close(parent);
			return -ENOENT;
		}
		
		if(dirent.ino == st.ino)
		{
			//Found it. Copy name out reversed.
			char *src = dirent.name + strlen(dirent.name) - 1;
			ssize_t used = 0;
			while(size > 0 && src >= dirent.name)
			{
				*buf = *src;
				buf++;
				src--;
				size--;
				used++;
			}
			px_fd_close(parent);
			return used;
		}
	}	
}


char *getcwd(char *buf, size_t size)
{
	if(buf == NULL)
	{
		//...I don't feel like handling this correctly
		if(size == 0)
			size = 4096;
		
		buf = malloc(size);
	}
	
	//Get FD referencing the current directory
	int fd = px_fd_find(-1, "");
	if(fd < 0)
	{
		errno = -fd;
		return NULL;
	}
	
	//Look up the directory in its parent. Then iterate up and look up the parent, etc.
	//Go until we find the root. Work with backwards strings so we advance in the buffer, then reverse at the end.
	char *buf_remain = buf;
	size_t size_remain = size;
	while(1)
	{
		//Get the next component - reversed.
		ssize_t result = _getcwd_name(fd, buf_remain, size_remain);
		if(result < 0)
		{
			px_fd_close(fd);
			errno = -result;
			return NULL;
		}
		
		buf_remain += result;
		size_remain -= result;
		if(size_remain <= 0) //Need room for the slash or NUL
		{
			px_fd_close(fd);
			errno = ERANGE;
			return NULL;
		}
		
		if(result > 0 && buf_remain[-1] == '/')
			break; //Got to root dir
		
		buf_remain[0] = '/';
		buf_remain++;
		size_remain--;
		if(size_remain <= 0) //Need room for the NUL
		{
			px_fd_close(fd);
			errno = ERANGE;
			return NULL;
		}
		
		int parent = px_fd_find(fd, "..");
		if(parent < 0)
		{
			px_fd_close(fd);
			errno = -parent;
			return NULL;
		}
		
		px_fd_close(fd);
		fd = parent;
	}
	
	px_fd_close(fd);
	
	//Reverse the buffer
	int ia = 0;
	int ib = strlen(buf) - 1;
	while(ia < ib)
	{
		char tmp = buf[ia];
		buf[ia] = buf[ib];
		buf[ib] = tmp;
		ia++;
		ib--;
	}
	
	//Success
	return buf;
}
