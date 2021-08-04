//termios.c
//Terminal control functions in libc
//Bryan E. Topp <betopp@betopp.com> 2021

#include <termios.h>
#include <px.h>
#include <errno.h>
#include <string.h>

int tcgetattr(int fd, struct termios *t)
{	
	int err = px_fd_ioctl(fd, PX_FD_IOCTL_GETATTR, t, sizeof(*t));
	if(err < 0)
	{
		errno = -err;
		return -1;
	}
	return 0;
}

int tcsetattr(int fd, int action, const struct termios *t)
{
	struct termios tcopy = {0};
	memcpy(&tcopy, t, sizeof(tcopy));
	tcopy.action = action;
	int err = px_fd_ioctl(fd, PX_FD_IOCTL_SETATTR, &tcopy, sizeof(tcopy));
	if(err < 0)
	{
		errno = -err;
		return -1;
	}
	return 0;
}

pid_t tcgetpgrp(int fd)
{
	int res = px_fd_ioctl(fd, PX_FD_IOCTL_GETPGRP, NULL, 0);
	if(res < 0)
	{
		errno = -res;
		return -1;
	}
	return res;
}

int tcsetpgrp(int fd, pid_t pgrp_id)
{
	int res = px_fd_ioctl(fd, PX_FD_IOCTL_SETPGRP, &pgrp_id, sizeof(pgrp_id));
	if(res < 0)
	{
		errno = -res;
		return -1;
	}
	return 0;
}

