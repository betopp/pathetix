//setlogin.c
//Temp implementation of setlogin in libc
//Bryan E. Topp <betopp@betopp.com> 2021

#include <errno.h>
#include <string.h>

//For now we're single-user so all processes have permission to "change their login name".
//So just keep this in memory.
char _login[32] = "root";

char *getlogin(void)
{
	return _login;
}

int getlogin_r(char *name, size_t len)
{
	if(strlen(_login) >= len)
		return ERANGE;
	
	strncpy(name, _login, len);
	return 0;
}

int setlogin(const char *name)
{
	if(strlen(name) >= sizeof(_login))
	{
		errno = EINVAL;
		return -1;
	}
	
	strncpy(_login, name, sizeof(_login)-1);
	return 0;
}
