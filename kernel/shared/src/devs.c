//devs.c
//Misc device drivers
//Bryan E. Topp <betopp@betopp.com> 2021

#include "devs.h"

ssize_t dev_null_read(int minor, void *buf, size_t size)
{
	(void)minor;
	(void)buf;
	(void)size;
	return 0;
}

ssize_t dev_null_write(int minor, const void *buf, size_t size)
{
	(void)minor;
	(void)buf;
	return size;
}

