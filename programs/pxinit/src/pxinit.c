//pxinit.c
//Init process
//Bryan E. Topp <betopp@betopp.com> 2021

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <sys/wait.h>

int main(int argc, const char **argv)
{
	(void)argc;
	(void)argv;
	
	//Todo - obviously this needs a fully-parallel task scheduling, system management, binary logging, and phone-home framework
	int con = open("/dev/con", O_RDWR);
	assert(con >= 0);
	const char *initstr = "pxinit " BUILDVERSION " built " BUILDDATE " by " BUILDUSER "\n";
	write(con, initstr, strlen(initstr));
	
	//Spawn shells forever
	while(1)
	{
		const char *spawnstr = "pxinit: launching /bin/oksh.\n";
		write(con, spawnstr, strlen(spawnstr));
		
		pid_t forked = fork();
		if(forked < 0)
		{
			//Error
			perror("fork");
			return -1;
		}
		else if(forked == 0)
		{
			//Child - exec the shell
			dup2(con, STDIN_FILENO);
			dup2(con, STDOUT_FILENO);
			dup2(con, STDERR_FILENO);
			execv("/bin/oksh", (char*[]){"oksh", 0});
			perror("execv");
			return -1;
		}

		//Parent - wait for child to exit
		while(1)
		{
			int status = 0;
			pid_t waited = wait(&status);
			if( (waited == forked) && (WIFEXITED(status)) )
			{
				//Shell exited, try again to spawn it
				break;
			}
		}
		
		const char *diedstr = "pxinit: shell died.\n";
		write(con, diedstr, strlen(diedstr));
	}
	
	return -1;
}

