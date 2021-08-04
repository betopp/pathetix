//mmlibc/include/mmbits/define_oflags.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _DEFINE_OFLAGS_H
#define _DEFINE_OFLAGS_H

//Exactly one of these may be specified at a time.
#define O_STAT 0 //"Open for stat" with no further permission requested.
#define O_RDONLY 1
#define O_WRONLY 2
#define O_RDWR 3
#define O_EXEC 4
#define O_SEARCH 4

//Bitwise OR of the above values.
#define O_ACCMODE 0x7

//Any combination of these flags may be used as well.
#define O_CREAT 64
#define O_EXCL 128
#define O_NOCTTY 256
#define O_TRUNC 512
#define O_APPEND 1024
#define O_NONBLOCK 2048
#define O_DIRECTORY 65536
#define O_NOFOLLOW 131072
#define O_CLOEXEC 524288
#define O_TTY_INIT (1<<24) //Linux doesn't define this...?

//These are the literal values that Linux uses... need to piece this apart.
#define O_DSYNC 4096
#define O_RSYNC 1052672
#define O_SYNC 1052672

#endif //_DEFINE_OFLAGS_H
