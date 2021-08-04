//mmlibc/include/mmbits/typedef_pthread.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _TYPEDEF_PTHREAD_H
#define _TYPEDEF_PTHREAD_H

//pthread_t is actually a pointer to the TLS for the thread.
typedef struct _tls_s* pthread_t;

#endif //_TYPEDEF_PTHREAD_H
