//confstr.c
//Implementation of confstr for libc
//Bryan E. Topp <betopp@betopp.com> 2021

#include <unistd.h>
#include <string.h>
#include <errno.h>

size_t confstr(int name, char *buf, size_t len)
{
	switch(name)
	{
		case _CS_PATH:
		{
			static const char *path = "/bin";
			strncpy(buf, "/bin", len);
			return strlen(path)+1;
		}
		default:
		{
			errno = EINVAL;
			return 0;
		}
	}
}
