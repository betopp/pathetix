//exit.c
//Exit and at-exit functions in libc
//Bryan E. Topp <betopp@betopp.com> 2021

#include <stdlib.h>
#include <errno.h>
#include <px.h>

typedef struct _atexit_s
{
	void (*function)(void);
	struct _atexit_s *next;
} _atexit_t;
_atexit_t *_atexit_list = NULL;

void _atexit_run(void)
{
	//Pop all at-exit functions off the list, calling the function and freeing each entry.
	while(_atexit_list != NULL)
	{
		_atexit_t *a = _atexit_list;
		_atexit_list = a->next;
		
		a->function();
		free(a);
	}
}

void exit(int status)
{
	_atexit_run();
	
	//Todo - flush streams, close output streams, delete temp files
	
	_Exit(status);
}

void _Exit(int status)
{
	px_exit(status, 0);
	
	//That shouldn't return, but we really want to crash if it does.
	while(1) { (*(volatile int*)0)++; }
}

int atexit(void (*function)(void))
{
	//Allocate new list entry
	_atexit_t *new_atexit = malloc(sizeof(_atexit_t));
	if(new_atexit == NULL)
	{
		errno = ENOMEM;
		return -1;
	}
	
	//Put it on the list
	new_atexit->function = function;
	new_atexit->next = _atexit_list;
	_atexit_list = new_atexit;
	
	return 0;
}

void abort(void)
{
	raise(SIGABRT);
}
