//ttyname.c
//ttyname for libc
//Bryan E. Topp <betopp@betopp.com> 2021

#include <unistd.h>
#include <px.h>

char _ttyname_buf[32];
char *ttyname(int fd)
{
	int result = ttyname_r(fd, _ttyname_buf, sizeof(_ttyname_buf) - 1);
	if(result == 0)
		return _ttyname_buf;
	else
		return NULL;
}

int ttyname_r(int fd, char *buf, size_t len)
{
	int result = px_fd_ioctl(fd, PX_FD_IOCTL_TTYNAME, buf, len);
	if(result < 0)
		return -result;
	
	return 0;
}

int isatty(int fd)
{
	int result = px_fd_ioctl(fd, PX_FD_IOCTL_ISATTY, NULL, 0);
	if(result > 0)
		return 1;
	else
		return 0;
}
