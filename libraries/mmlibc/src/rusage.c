//rusage.c
//Resource usage query in libc
//Bryan E. Topp <betopp@betopp.com> 2021

#include <sys/resource.h>
#include <errno.h>
#include <px.h>

int getrusage(int who, struct rusage *rusage)
{
	px_rusage_t r = {0};
	int err = px_rusage(who, &r, sizeof(r));
	if(err < 0)
	{
		errno = -err;
		return -1;
	}
	
	rusage->ru_utime.tv_sec = r.utime_sec;
	rusage->ru_utime.tv_usec = r.utime_usec;
	rusage->ru_stime.tv_sec = r.stime_sec;
	rusage->ru_stime.tv_usec = r.stime_usec;
	return 0;
}
