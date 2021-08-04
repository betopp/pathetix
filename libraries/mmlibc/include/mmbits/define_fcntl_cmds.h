//mmlibc/include/mmbits/define_fcntl_cmds.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _DEFINE_FCNTL_CMDS_H
#define _DEFINE_FCNTL_CMDS_H

//Defined like Linux
#define F_DUPFD 0
#define F_DUPFD_CLOEXEC 1030
#define F_GETFD 1
#define F_SETFD 2
#define F_GETFL 3
#define F_SETFL 4
#define F_GETLK 5
#define F_SETLK 6
#define F_SETLKW 7
#define F_GETOWN 9
#define F_SETOWN 8

#endif //_DEFINE_FCNTL_CMDS_H
