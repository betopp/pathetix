//mmlibc/include/mmbits/struct_stat.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _STRUCT_STAT_H
#define _STRUCT_STAT_H

#include <mmbits/typedef_dev.h>
#include <mmbits/typedef_ino.h>
#include <mmbits/typedef_mode.h>
#include <mmbits/typedef_nlink.h>
#include <mmbits/typedef_uid.h>
#include <mmbits/typedef_gid.h>
#include <mmbits/typedef_off.h>
#include <mmbits/struct_timespec.h>
#include <mmbits/typedef_blksize.h>
#include <mmbits/typedef_blkcnt.h>

struct stat
{
	dev_t st_dev;            //Device containing the file
	ino_t st_ino;            //File index on device
	mode_t st_mode;          //File mode
	nlink_t st_nlink;        //Number of hard links to the file
	uid_t st_uid;            //User ID of owner
	gid_t st_gid;            //Group ID of owner
	dev_t st_rdev;           //Device ID, if special-file
	off_t st_size;           //File size
	struct timespec st_atim; //Last data access
	struct timespec st_mtim; //Last data modification
	struct timespec st_ctim; //Last status change
	blksize_t st_blksize;    //Preferred IO block size
	blkcnt_t st_blocks;      //Number of blocks allocated
};

#endif //_STRUCT_STAT_H
