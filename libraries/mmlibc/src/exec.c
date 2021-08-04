//exec.c
//Exec and friends in standard library
//Bryan E. Topp <betopp@betopp.com> 2021

#include <unistd.h>
#include <px.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>

int fexecve(int fd, char *const argv[], char *const envp[])
{
	int result = px_fd_exec(fd, argv, envp);
	
	//The above syscall should only return on error.
	assert(result < 0);
	errno = -result;
	return -1;
}

int execve(const char *path, char *const argv[], char *const envp[])
{
	int fd = open(path, O_EXEC | O_CLOEXEC);
	if(fd < 0)
		return -1;
	
	return fexecve(fd, argv, envp);
}

int execv(const char *path, char *const argv[])
{
	extern char **environ;
	return execve(path, argv, environ);
}
