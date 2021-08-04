//libcstubs.h
//libc-like functions in kernel
//Bryan E. Topp <betopp@betopp.com> 2021

#include "libcstubs.h"
#include "kspace.h"

int memcmp(const void *s1, const void *s2, size_t n)
{
	const unsigned char *b1 = (const unsigned char *)s1;
	const unsigned char *b2 = (const unsigned char *)s2;
	for(size_t bb = 0; bb < n; bb++)
	{
		if(b1[bb] != b2[bb])
		{
			return b1[bb] - b2[bb];
		}
	}
	return 0;
}

void *memset(void *s, int c, size_t n)
{
	unsigned char *b = (unsigned char*)s;
	for(size_t bb = 0; bb < n; bb++)
	{
		b[bb] = (unsigned char)c;
	}
	return s;
}

void *memcpy(void *dest, const void *src, size_t n)
{
	unsigned char *destc = (unsigned char*)dest;
	const unsigned char *srcc = (const unsigned char*)src;
	for(size_t bb = 0; bb < n; bb++)
	{
		destc[bb] = srcc[bb];
	}
	return dest;
}

size_t strlen(const char *s)
{
	size_t n = 0;
	while(s[n] != '\0')
	{
		n++;
	}
	return n;
}

char *strncpy(char *dest, const char *src, size_t n)
{
	size_t bb = 0;
	while(bb < n && src[bb] != '\0')
	{
		dest[bb] = src[bb];
		bb++;
	}
	while(bb < n)
	{
		dest[bb] = '\0';
		bb++;
	}
	return dest;
}

int strcmp(const char *s1, const char *s2)
{
	while(1)
	{
		if(*s1 != *s2)
			return *s1 - *s2;
		
		if(*s1 == '\0')
			return 0;
		
		s1++;
		s2++;
	}
}

int strncmp(const char *s1, const char *s2, size_t n)
{
	for(size_t ii = 0; ii < n; ii++)
	{
		if(*s1 != *s2)
			return *s1 - *s2;
		
		if(*s1 == '\0')
			return 0;
		
		s1++;
		s2++;
	}
	
	return 0;
}

char *strchr(const char *s, int c)
{
	const char *ii = s;
	while(1)
	{
		if(*ii == c)
			return (char*)ii; //Note - has to handle case where c == '\0'. Note also: const-incorrect by POSIX.
		
		if(*ii == '\0')
			return NULL;
		
		ii++;
	}
}
