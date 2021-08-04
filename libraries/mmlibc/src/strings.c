//strings.c
//Extra string functions for DUCK's libc
//Bryan E. Topp <betopp@betopp.com> 2020

#include <limits.h>
#include <stdint.h>
#include <strings.h>
#include <ctype.h>


int strcasecmp(const char *s1, const char *s2)
{
	return strncasecmp(s1, s2, SIZE_MAX);
}

int strncasecmp(const char *s1, const char *s2, size_t n)
{
	for(size_t bb = 0; bb < n; bb++)
	{		
		char s1_lower = tolower(s1[bb]);
		char s2_lower = tolower(s2[bb]);
		if(s1_lower != s2_lower)
		{
			return s1_lower - s2_lower;
		}

		if(s1_lower == '\0' || s2_lower == '\0')
			break;
	}
	
	return 0;
}

int ffs(int i)
{
	for(int bb = 0; bb < 64; bb++)
	{
		if(i & (1<<bb))
			return bb+1;
	}
	return 0;
}
