//mmlibc/include/mmbits/define_atflags.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _DEFINE_ATFLAGS_H
#define _DEFINE_ATFLAGS_H

#define AT_FDCWD (-100) //I don't know why Linux does this but... ok.

#define AT_EACCESS 512
#define AT_SYMLINK_NOFOLLOW 256
#define AT_SYMLINK_FOLLOW 1024
#define AT_REMOVEDIR 512

#endif //_DEFINE_ATFLAGS_H
