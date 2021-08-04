//clocks.c
//Time counting in libc
//Bryan E. Topp <betopp@betopp.com> 2021

#include <time.h>
#include <errno.h>
#include <px.h>


int clock_gettime(clockid_t clock_id, struct timespec *tp)
{
	(void)clock_id;
	(void)tp;
	
	errno = ENOSYS;
	return -1;
}

unsigned int sleep(unsigned int seconds)
{
	int err = px_nanosleep(seconds * 1000000000l);
	if(err < 0)
		return seconds;
	else
		return 0;
}

unsigned int alarm(unsigned int seconds)
{
	(void)seconds;
	return 0;
}
