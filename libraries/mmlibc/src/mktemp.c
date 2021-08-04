//mktemp.c
//Temporary file functions in libc
//Bryan E. Topp <betopp@betopp.com> 2021

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

//Returns a random alphanumeric character for use in a temporary filename.
static char _mktemp_rand(void)
{
	int r = rand() % 62;
	if(r < 26)
		return 'a' + r;
	else if(r < 52)
		return 'A' + (r - 26);
	else
		return '0' + (r - 52);
}

char *mktemp(char *tpl)
{
	//Handle NULL or zero-length strings trivially
	if(tpl == NULL || *tpl == '\0')
		return NULL;
	
	//Work backwards from the last character, replacing trailing X with random alphanum.
	int replaced = 0;
	char *cc = tpl + strlen(tpl) - 1;
	while(cc >= tpl)
	{
		if(*cc != 'X')
			break;
		
		*cc = _mktemp_rand();		
		cc--;
		replaced++;
	}
	
	if(replaced > 0)
		return tpl;
	else
		return NULL;
}

int mkstemp(char *tpl)
{
	char *name = mktemp(tpl); //Probably in the same buffer, ultimately
	if(name == NULL)
	{
		errno = EINVAL;
		return -1;
	}
	
	int fd = open(tpl, O_CREAT | O_EXCL, 0600);
	return fd; //open sets errno and returns -1 on failure
}
