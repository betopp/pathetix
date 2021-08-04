//id.c
//Process/user ID functions for libc
//Bryan E. Topp <betopp@betopp.com> 2021

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <px.h>

pid_t getpid(void)
{
	return px_getpid();
}

pid_t getppid(void)
{
	return px_getppid();
}

int gethostname(char *name, size_t namelen)
{
	static const char * const hostname = "localhost";
	if(namelen < strlen(hostname))
	{
		errno = ENAMETOOLONG;
		return -1;
	}
	
	strncpy(name, hostname, namelen);
	return 0;
}

pid_t getpgrp(void)
{
	return getpgid(0);
}

pid_t getpgid(pid_t pid)
{
	pid_t retval = px_getpgid(pid);
	if(retval < 0)
	{
		errno = -retval;
		return -1;
	}
	return retval;
}

int setpgid(pid_t pid, pid_t pgrp)
{
	int result = px_setpgid(pid, pgrp);
	if(result < 0)
	{
		errno = -result;
		return -1;
	}
	return 0;
}

pid_t getsid(pid_t pid)
{
	//one session
	(void)pid;
	return 0;
}

gid_t getgid(void)
{
	//one group
	return 0;
}

gid_t getegid(void)
{
	//one group
	return 0;
}

uid_t getuid(void)
{
	//one user
	return 0;
}

uid_t geteuid(void)
{
	//one user
	return 0;
}

int setgid(gid_t gid)
{
	if(gid != 0)
	{
		errno = ENOSYS;
		return -1;
	}
	return 0;
}

int setegid(gid_t gid)
{
	if(gid != 0)
	{
		errno = ENOSYS;
		return -1;
	}
	return 0;
}

int setuid(uid_t uid)
{
	if(uid != 0)
	{
		errno = ENOSYS;
		return -1;
	}
	return 0;
}

int seteuid(uid_t uid)
{
	if(uid != 0)
	{
		errno = ENOSYS;
		return -1;
	}
	return 0;
}
