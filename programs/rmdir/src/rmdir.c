//rmdir.c
//Directory deletion utility
//Bryan E. Topp <betopp@betopp.com> 2021

#include <unistd.h>
#include <stdio.h>

int main(int argc, const char **argv)
{
	for(int aa = 1; aa < argc; aa++)
	{
		int result = rmdir(argv[aa]);
		if(result < 0)
		{
			perror("rmdir");
			return -1;
		}
	}
	return 0;
}
