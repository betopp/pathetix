//dlibc/src/common/string.c
//Entry points for string.h functions in MuKe's libc
//Bryan E. Topp <betopp@betopp.com> 2021

#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

//Define error names
#define NUM_ERR_NAMES 256
static const char *_err_names[NUM_ERR_NAMES] = 
{
	[0] = "No error",
	[E2BIG] = "Argument list too long",
	[EACCES] = "Permission denied",
	[EADDRINUSE] = "Address in use",
	[EADDRNOTAVAIL] = "Address not available",
	[EAFNOSUPPORT] = "Address family not supported",
	[EAGAIN] = "Resource unavailable, try again",
	[EALREADY] = "Connection already in progress",
	[EBADF] = "Bad file descriptor",
	[EBADMSG] = "Bad message",
	[EBUSY] = "Device or resource busy",
	[ECANCELED] = "Operation canceled",
	[ECHILD] = "No child processes",
	[ECONNABORTED] = "Connection aborted",
	[ECONNREFUSED] = "Connection refused",
	[ECONNRESET] = "Connection reset",
	[EDEADLK] = "Resource deadlock would occur",
	[EDESTADDRREQ] = "Destination address required",
	[EDOM] = "Mathematics argument out of domain of function",
	[EDQUOT] = "Reserved",
	[EEXIST] = "File exists",
	[EFAULT] = "Bad address",
	[EFBIG] = "File too large",
	[EHOSTUNREACH] = "Host is unreachable",
	[EIDRM] = "Identifier removed",
	[EILSEQ] = "Illegal byte sequence",
	[EINPROGRESS] = "Operation in progress",
	[EINTR] = "Interrupted function",
	[EINVAL] = "Invalid argument",
	[EIO] = "I/O error",
	[EISCONN] = "Socket is connected",
	[EISDIR] = "Is a directory",
	[ELOOP] = "Too many levels of symbolic links",
	[EMFILE] = "File descriptor value too large",
	[EMLINK] = "Too many links",
	[EMSGSIZE] = "Message too large",
	[EMULTIHOP] = "Reserved",
	[ENAMETOOLONG] = "Filename too long",
	[ENETDOWN] = "Network is down",
	[ENETRESET] = "Connection aborted by network",
	[ENETUNREACH] = "Network unreachable",
	[ENFILE] = "Too many files open in system",
	[ENOBUFS] = "No buffer space available",
	[ENODATA] = "No message is available on the STREAM head read queue",
	[ENODEV] = "No such device",
	[ENOENT] = "No such file or directory",
	[ENOEXEC] = "Executable file format error",
	[ENOLCK] = "No locks available",
	[ENOLINK] = "Reserved",
	[ENOMEM] = "Not enough space",
	[ENOMSG] = "No message of the desired type",
	[ENOPROTOOPT] = "Protocol not available",
	[ENOSPC] = "No space left on device",
	[ENOSR] = "No STREAM resources",
	[ENOSTR] = "Not a STREAM",
	[ENOSYS] = "Functionality not supported",
	[ENOTCONN] = "The socket is not connected",
	[ENOTDIR] = "Not a directory or a symbolic link to a directory",
	[ENOTEMPTY] = "Directory not empty",
	[ENOTRECOVERABLE] = "State not recoverable",
	[ENOTSOCK] = "Not a socket",
	[ENOTSUP] = "Not supported",
	[ENOTTY] = "Inappropriate I/O control operation",
	[ENXIO] = "No such device or address",
	//[EOPNOTSUPP] = "Operation not supported on socket (may be the same value as [ENOTSUP])",
	[EOVERFLOW] = "Value too large to be stored in data type",
	[EOWNERDEAD] = "Previous owner died",
	[EPERM] = "Operation not permitted",
	[EPIPE] = "Broken pipe",
	[EPROTO] = "Protocol error",
	[EPROTONOSUPPORT] = "Protocol not supported",
	[EPROTOTYPE] = "Protocol wrong type for socket",
	[ERANGE] = "Result too large",
	[EROFS] = "Read-only file system",
	[ESPIPE] = "Invalid seek",
	[ESRCH] = "No such process",
	[ESTALE] = "Reserved",
	[ETIME] = "Stream ioctl() timeout",
	[ETIMEDOUT] = "Connection timed out",
	[ETXTBSY] = "Text file busy",
	//[EWOULDBLOCK] = "Operation would block (may be the same value as [EAGAIN])",
	[EXDEV] = "Cross-device link",
};
//static const char *err_generic_format = "Unknown error %d";


void *memccpy(void *dst, const void *src, int c, size_t n)
{
	const unsigned char *src_bytes = src;
	unsigned char *dst_bytes = dst;	
	for(size_t i = 0; i < n; i++)
	{
		dst_bytes[i] = src_bytes[i];
		if(dst_bytes[i] == c)
			return &(dst_bytes[i+1]);
	}
	
	return NULL;
}

void *memchr(const void *s, int c, size_t n)
{
	const unsigned char *s_bytes = s;
	for(size_t i = 0; i < n; i++)
	{
		if(s_bytes[i] == c)
			return (void*)(&(s_bytes[i]));
	}
	
	return NULL;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
	const unsigned char *s1_bytes = s1;
	const unsigned char *s2_bytes = s2;
	for(size_t i = 0; i < n; i++)
	{
		int diff = s1_bytes[i] - s2_bytes[i];
		if(diff != 0)
			return diff;
	}
	
	return 0;
}

void *memcpy(void *dst, const void *src, size_t n)
{
	const unsigned char *src_bytes = src;
	unsigned char *dst_bytes = dst;
	
	//Do as many 64-bit copies as we can
	while(n >= 8)
	{
		*((uint64_t*)dst_bytes) = *((uint64_t*)src_bytes);
		src_bytes += 8;
		dst_bytes += 8;
		n -= 8;
	}
	
	//Finish off with 1-byte copies
	while(n > 0)
	{
		*dst_bytes = *src_bytes;
		dst_bytes++;
		src_bytes++;
		n--;
	}
	
	return dst;
}

void *memmove(void *dst, const void *src, size_t n)
{
	const unsigned char *src_bytes = src;
	unsigned char *dst_bytes = dst;
	
	if(dst == src || n == 0)
	{
		//No copy needed
	}
	else if(dst > src)
	{
		for(size_t i = n-1; 1; i--)
		{
			dst_bytes[i] = src_bytes[i];
			if(i == 0)
				break;
		}
	}
	else
	{
		for(size_t i = 0; i < n; i++)
		{
			dst_bytes[i] = src_bytes[i];
		}
	}
	
	return dst;
}


void *memset(void *s, int c, size_t n)
{	
	unsigned char *s_bytes = s;
	unsigned char charval = (unsigned char)c;
	
	//Work up to a good alignment with byte writes
	while( n > 0 && (((uintptr_t)s_bytes) % 8) != 0 )
	{
		*s_bytes = charval;
		s_bytes++;
		n--;
	}
	
	//Do 64-bit writes
	uint64_t eighttimes = charval;
	eighttimes |= eighttimes << 8;
	eighttimes |= eighttimes << 16;
	eighttimes |= eighttimes << 32;
	while( n >= 8 )
	{
		*((uint64_t*)(s_bytes)) = eighttimes;
		s_bytes += 8;
		n -= 8;
	}
	
	//Finish off with byte writes
	while(n > 0)
	{
		*s_bytes = charval;
		s_bytes++;
		n--;
	}
	
	return s;
}

char *strcat(char *dst, const char *src)
{
	size_t lsrc = strlen(src);
	size_t ldst = strlen(dst);
	for(size_t i = 0; i < lsrc; i++)
	{
		dst[ldst + i] = src[i];
	}
	dst[lsrc+ldst] = '\0';
	return dst;
}

char *strchr(const char *s, int c)
{
	if(c == '\0')
		return (char*)(s + strlen(s));
	
	for(size_t i = 0; s[i] != '\0'; i++)
	{
		if(s[i] == c)
			return (char*)&(s[i]);
	}	
	return NULL;
}

int strcmp(const char *s1, const char *s2)
{
	for(size_t i = 0; (s1[i] != '\0') || (s2[i] != '\0'); i++)
	{
		int diff = s1[i] - s2[i];
		if(diff != 0)
			return diff;
	}
	
	return 0;
}

//int strcoll(const char *s1, const char *s2)
//{
	//I don't care about locales.
//	errno = ENOSYS;
//	return strcmp(s1, s2);
//}

char *strcpy(char *dst, const char *src)
{
	size_t i;
	for(i = 0; src[i] != '\0'; i++)
	{
		dst[i] = src[i];
	}
	dst[i] = '\0';
	
	return dst;
}

size_t strcspn(const char *s, const char *reject)
{
	size_t i;
	for(i = 0; s[i] != '\0'; i++)
	{
		for(size_t j = 0; reject[j] != '\0'; j++)
		{
			if(s[i] == reject[j])
				return i;
		}
	}
	return i;
}

char *strdup(const char *s)
{
	char *newstring = (char*)malloc(strlen(s)+1);
	strcpy(newstring, s);
	return newstring;
}

static char _strerror_non_reentrant_buffer[64];
char *strerror(int errnum)
{
	_strerror_non_reentrant_buffer[63] = '\0';
	strerror_r(errnum, _strerror_non_reentrant_buffer, 63);
	return _strerror_non_reentrant_buffer;
}

int strerror_r(int errnum, char *buf, size_t buflen)
{
	if( (errnum >= 0) && (errnum < NUM_ERR_NAMES) && (_err_names[errnum] != 0) )
	{
		strncpy(buf, _err_names[errnum], buflen);
	}
	else
	{
		strncpy(buf, "unimplemented", buflen);
		//assert(0);
		//snprintf(buf, buflen, err_generic_format, errnum);
	}
	
	return 0;
}

size_t strlen(const char *s)
{
	size_t i;
	for(i = 0; s[i] != '\0'; i++)
	{
		
	}
	return i;
}

char *strncat(char *dst, const char *src, size_t n)
{
	size_t lsrc = strnlen(src, n);
	size_t ldst = strlen(dst);
	for(size_t i = 0; i < lsrc; i++)
	{
		dst[ldst + i] = src[i];
	}
	dst[lsrc+ldst] = '\0';
	return dst;	
}

int strncmp(const char *s1, const char *s2, size_t n)
{
	for(size_t i = 0; (i < n) && (s1[i] != '\0' || s2[i] != '\0'); i++)
	{
		int diff = s1[i] - s2[i];
		if(diff != 0)
			return diff;
	}
	return 0;
}

char *strncpy(char *dst, const char *src, size_t n)
{
	size_t i;
	for(i = 0; i < n; i++)
	{
		dst[i] = src[i];
		if(src[i] == '\0')
			break;
	}
	
	for( ; i < n; i++)
	{
		dst[i] = '\0';
	}
	
	return dst;
}

size_t strnlen(const char *s, size_t n)
{
	size_t i;
	for(i = 0; i < n; i++)
	{
		if(s[i] == '\0')
			break;
	}
	return i;
}

char *strpbrk(const char *s, const char *accept)
{
	for(size_t i = 0; s[i] != '\0'; i++)
	{
		for(size_t j = 0; accept[j] != '\0'; j++)
		{
			if(s[i] == accept[j])
				return (char*)&(s[i]);
		}
	}
	return NULL;
}

char *strrchr(const char *s, int c)
{
	for(size_t i = strlen(s); 1; i--)
	{
		if(s[i] == c)
			return (char*)&(s[i]);
		
		if(i == 0)
			break;
	}
	
	return NULL;
}

size_t strspn(const char *s, const char *accept)
{
	size_t i;
	for(i = 0; s[i] != '\0'; i++)
	{
		size_t j;
		for(j = 0; accept[j] != '\0'; j++)
		{
			if(s[i] == accept[j])
				break;
		}
		
		if(accept[j] == '\0')
			return i;
	}
	return i;
}

char *strstr(const char *haystack, const char *needle)
{
	for(size_t i = 0; haystack[i] != '\0'; i++)
	{
		size_t j;
		for(j = 0; needle[j] != '\0'; j++)
		{
			if(haystack[i+j] != needle[j])
				break;
		}
		if(needle[j] == '\0')
			return (char*)&(haystack[i]);
	}
	return NULL;
}

static char *_strtok_non_reentrant_charptr = NULL;
char *strtok(char *str, const char *delim)
{
	return strtok_r(str, delim, &_strtok_non_reentrant_charptr);
}

char *strtok_r(char *str, const char *delim, char **saveptr)
{
	if(str == NULL)
		str = *saveptr;
	
	if(str == NULL || str[0] == '\0')
		return NULL;
	
	//We may have to skip over delimiters at the beginning of input -
	//so keep track of where the actual token begins.
	char *tokenstart = str;
	
	//Trivial search for delimiter characters
	for(char *sp = str; *sp != '\0'; sp++)
	{
		for(const char *dp = delim; *dp != '\0'; dp++)
		{
			if(*sp == *dp)
			{
				//Found match between string and delimiters.
				
				//Chop string.
				*sp = '\0';
				
				//If the delimiter is at the beginning of input, ignore and keep going
				if(sp == tokenstart)
				{
					tokenstart = sp + 1;
					continue;
				}
				
				//Otherwise, return the token now that we've hacked off its end.
				*saveptr = sp + 1;
				return tokenstart;
			}
		}
	}
		
	//No delimiters at all...
	*saveptr = NULL;
	return str;
}

//size_t strxfrm(char *dst, const char *src, size_t n)
//{
	//Still don't care about locales
//	errno = ENOSYS;
	
//	if(n == 0)
//		return 0;
	
//	strncpy(dst, src, n);
//	return strnlen(dst, n);
//}

void perror(const char *string)
{
	int toprint = errno;
	const char *errname = NULL;
	if(toprint < 0 || toprint >= NUM_ERR_NAMES)
		errname = "Bad error number";
	else
		errname = _err_names[toprint];
		
	if(string != NULL && *string != '\0')
	{
		fprintf(stderr, "%s: %s\n", string, errname);
	}
	else
	{
		fprintf(stderr, "%s\n", errname);
	}
}


