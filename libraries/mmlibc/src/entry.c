//entry.c
//libc entry point
//Bryan E. Topp <betopp@betopp.com> 2021

#include <stdlib.h>
#include <string.h>
#include <signal.h>

void _libc_entry(void)
{
	//Kernel provides argv/envp at static locations
	char **argv = *(char***)(0x1000);
	char **envp = *(char***)(0x1000 + sizeof(char**));
	
	//Set aside initial environment location
	extern char **environ;
	environ = envp;
	
	//Count argv
	int argc = 0;
	while(argv[argc] != NULL)
		argc++;
	
	//Set signal actions to default
	for(int ss = 0; ss < 64; ss++)
	{
		signal(ss, SIG_DFL);
	}
	
	//Call main
	extern int main();
	int main_returned = main(argc, argv, envp);
	
	//If main returns, call exit with its return value.
	exit(main_returned);
	
	//Exit shouldn't return
	_Exit(main_returned);
	abort();
	while(1){}
}
