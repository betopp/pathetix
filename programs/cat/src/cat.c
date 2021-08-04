//cat.c
//Concatenation utility
//Bryan E. Topp <betopp@betopp.com> 2021

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

char buf[4096];

void do_fd(int fd)
{
	while(1)
	{
		ssize_t nread = read(fd, buf, sizeof(buf));
		if(nread == 0)
		{
			//Done
			break;
		}
		
		if(nread < 0)
		{
			perror("read");
			exit(-1);
		}
		
		ssize_t nwrite = write(STDOUT_FILENO, buf, nread);
		if(nwrite != nread)
		{
			perror("write");
			exit(-1);
		}
	}
}

int main(int argc, const char **argv)
{
	if(argc < 2)
	{
		do_fd(STDIN_FILENO);
		return 0;
	}
	
	for(int aa = 1; aa < argc; aa++)
	{
		int fd = open(argv[aa], O_RDONLY);
		if(fd < 0)
		{
			perror("open");
			exit(-1);
		}
		
		do_fd(fd);
		
		close(fd);
	}
	
	return 0;
}
