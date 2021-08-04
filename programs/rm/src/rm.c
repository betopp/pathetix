//rm.c
//File deletion utility
//Bryan E. Topp <betopp@betopp.com> 2021

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, const char **argv)
{
	for(int aa = 1; aa < argc; aa++)
	{
		int result = unlink(argv[aa]);
		if(result < 0)
		{
			perror("unlink");
			return -1;
		}
	}
	return 0;
}
