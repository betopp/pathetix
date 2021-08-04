//mmlibc/include/tls.h
//Thread-local storage for MMK's libc
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _TLS_H
#define _TLS_H

#include <stdint.h>
#include <signal.h>

//Data in thread-local storage
typedef struct _tls_s
{
	//Pointer back to this TLS structure.
	//Placed at the beginning so we can find GS-base by reading [GS:0].
	struct _tls_s *self;
	
	//Thread-local errno value, for returning errors in standard library
	int errno;
	
	//Current signal actions for this thread
	struct sigaction sigactions[64];
	
} _tls_t;

//Returns pointer to thread-local storage.
_tls_t *_tls();

#endif //_TLS_H
