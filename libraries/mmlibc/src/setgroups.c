//setgroups.c
//Group changing for libc
//Bryan E. Topp <betopp@betopp.com> 2021

#include <unistd.h>

int setgroups(size_t ngroups, const gid_t *groups)
{
	//Single-user implementation for now...
	(void)ngroups;
	(void)groups;
	return 0;
}
