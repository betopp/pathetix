//basename.c
//basename implementation for libc
//Bryan E. Topp <betopp@betopp.com> 2021

#include <libgen.h>
#include <stddef.h>
#include <string.h>

char *basename(char *path)
{
	//Handle empty case
	if((path == NULL) || (strlen(path) < 1))
	{
		static char dot[2] = {'.', '\0'};
		return dot;
	}
	
	//Delete trailing slashes
	char *end = path + strlen(path);
	while(end > path)
	{
		if(*end == '/' || *end == '\0')
			*end = '\0';
		else
			break;
		
		end--;
	}
	
	if(path[0] == '/' && path[1] == '\0')
		return path; //Path was entirely slashes
	
	//Advance through slashes until none remain
	while(1)
	{
		char *slash = strchr(path, '/');
		if(slash == NULL)
			return path;
		
		path = slash;
		while(*path == '/')
			path++;
	}
}
