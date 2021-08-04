//alloc.c
//Shim for malloc that aborts on failure to allocate RAM
//Bryan E. Topp <betopp@betopp.com> 2021

#include "alloc.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void *alloc_mandatory(size_t size)
{
	void *result = calloc(1, size);
	if(result == NULL)
	{
		perror("calloc");
		abort();
	}
	return result;
}

void *realloc_mandatory(void *ptr, size_t size)
{
	void *result = realloc(ptr, size);
	if(result == NULL)
	{
		perror("realloc");
		abort();
	}
	return result;
}

char *strdup_mandatory(const char *str)
{
	char *result = strdup(str);
	if(result == NULL)
	{
		perror("strdup");
		abort();
	}
	return result;
}
