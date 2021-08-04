//mmlibc/include/mmbits/struct_sigevent.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _STRUCT_SIGEVENT_H
#define _STRUCT_SIGEVENT_H

#include <mmbits/union_sigval.h>
#include <mmbits/typedef_pthread_attr.h>

struct sigevent
{
	int sigev_notify; //Notification type
	int sigev_signo; //Signal number
	int sigev_value; //Signal value
	void (*sigev_notify_function)(union sigval); //Notification function
	pthread_attr_t *sigev_notify_attributes; //Notification attributes
};

#endif //_STRUCT_SIGEVENT_H
