//devs.h
//Misc device drivers
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef DEVS_H
#define DEVS_H

#include <sys/types.h>

//Implementation of /dev/null
ssize_t dev_null_read(int minor, void *buf, size_t size);
ssize_t dev_null_write(int minor, const void *buf, size_t size);


#endif //DEVS_H
