//sys/ioctl.h
//ioctl entry for libc
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef _IOCTL_H
#define _IOCTL_H

//POSIX tries to define ioctl in "stropts.h" but literally everyone seems to expect sys/ioctl.h.
int ioctl(int fd, unsigned long request, ...);

//OKSH needs this
#define TIOCGWINSZ 1

#endif //_IOCTL_H
