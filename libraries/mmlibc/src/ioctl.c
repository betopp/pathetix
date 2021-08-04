//ioctl.c
//IO control wrapper
//Bryan E. Topp <betopp@betopp.com 2021

#include <sys/ioctl.h>
#include <errno.h>
#include <stdarg.h>
#include <px.h>

int ioctl(int fd, unsigned long request, ...)
{
	va_list ap;
	va_start(ap, request);
	void *p3 = va_arg(ap, void *);
	va_end(ap);
	
	//Todo - table of requests that are used in the libc with the appropriate parameter sizes.
	
	int result = px_fd_ioctl(fd, request, p3, 0);
	if(result < 0)
	{
		errno = -result;
		return -1;
	}
	return result;
}

