//env.c
//getenv, setenv, and friends
//Bryan E. Topp <betopp@betopp.com> 2021

#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>

char **environ = NULL;
static bool _environ_dynamic = false;

char *getenv(const char *name)
{
	if(name == NULL || environ == NULL)
		return NULL;
	
	size_t namelen = strlen(name);
	for(int ee = 0; environ[ee] != NULL; ee++)
	{
		//Look for an environment string that begins with the name and then '='
		if(strlen(environ[ee]) < namelen + 1)
			continue;
		
		if(strncmp(environ[ee], name, namelen) != 0)
			continue;
		
		if(environ[ee][namelen] != '=')
			continue;
		
		//Return just the part after the '='
		return environ[ee] + namelen + 1;
	}
	
	//Didn't find it
	return NULL;
}

int setenv(const char *name, const char *value, int overwrite)
{
	//Allocate the array of pointers to env strings, if we don't have one.
	//The kernel gives us one but we shouldn't modify that - first time we want to, make a copy.
	if( (environ == NULL) || (_environ_dynamic == false) )
	{
		char **old_environ = environ;
		size_t old_environ_count = 0;
		if(old_environ != NULL)
		{
			while(old_environ[old_environ_count] != NULL)
				old_environ_count++;
		}
		
		environ = (char**)malloc(sizeof(char*) * (old_environ_count + 1));
		if(environ == NULL)
		{
			environ = old_environ;
			errno = ENOMEM;
			return -1;
		}
		
		for(int ee = 0; old_environ[ee] != NULL; ee++)
		{
			environ[ee] = malloc(strlen(old_environ[ee]) + 1);
			if(environ[ee] == NULL)
			{
				while(ee > 0)
				{
					ee--;
					free(environ[ee]);
				}
				environ = old_environ;
				errno = ENOMEM;
				return -1;
			}
			
			strcpy(environ[ee], old_environ[ee]);
		}
		environ[old_environ_count] = NULL;
		_environ_dynamic = true;
	}
	
	//Validate name/value parameters
	if( (name == NULL) || (strchr(name, '=') != NULL) )
	{
		errno = EINVAL;
		return -1;
	}
	
	if( (value == NULL) || (strchr(value, '=') != NULL) )
	{
		errno = EINVAL;
		return -1;
	}
	
	//See if there's an existing env string to replace
	size_t namelen = strlen(name);
	size_t valuelen = strlen(value);
	int ee;
	for(ee = 0; environ[ee] != NULL; ee++)
	{
		if(strlen(environ[ee]) < namelen + 1)
			continue;
		
		if(strncmp(environ[ee], name, namelen) != 0)
			continue;
		
		if(environ[ee][namelen] != '=')
			continue;
		
		//Found it - overwrite or abort, depending on parameter
		if(overwrite)
		{
			environ[ee] = realloc(environ[ee], namelen + 1 + valuelen + 1);
			memcpy(environ[ee], name, namelen);
			memcpy(environ[ee] + namelen, "=", 1);
			memcpy(environ[ee] + namelen + 1, value, valuelen);
			memcpy(environ[ee] + namelen + 1 + valuelen, "\0", 1);
		}
		return 0;
	}
	
	//Need to insert another string.
	
	//Expand the array of env string pointers.
	//ee at this point is the index of the NULL terminator in the old pointer array.
	//So ee+1 is the current number of pointers in environ[], and ee+2 is the new amount.
	environ = realloc(environ, sizeof(char*) * (ee + 2));
	
	//Add the new entry
	environ[ee] = malloc(namelen + 1 + valuelen + 1);
	memcpy(environ[ee], name, namelen);
	memcpy(environ[ee] + namelen, "=", 1);
	memcpy(environ[ee] + namelen + 1, value, valuelen);
	memcpy(environ[ee] + namelen + 1 + valuelen, "\0", 1);

	//Terminate the array
	environ[ee+1] = NULL;
	
	return 0;
}
