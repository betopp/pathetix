//mmlibc/include/sys/stat.h
//stat()-related declarations for MMK's libc.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _SYS_STAT_H
#define _SYS_STAT_H

#include <mmbits/struct_stat.h>

#include <mmbits/typedef_blkcnt.h>
#include <mmbits/typedef_blksize.h>
#include <mmbits/typedef_dev.h>
#include <mmbits/typedef_ino.h>
#include <mmbits/typedef_mode.h>
#include <mmbits/typedef_nlink.h>
#include <mmbits/typedef_uid.h>
#include <mmbits/typedef_gid.h>
#include <mmbits/typedef_off.h>
#include <mmbits/typedef_time.h>

#include <mmbits/struct_timespec.h>

#define st_atime st_atim.tv_sec
#define st_ctime st_ctim.tv_sec
#define st_mtime st_mtim.tv_sec

#include <mmbits/define_modes.h>
#include <mmbits/define_mode_flags.h>
#include <mmbits/define_mode_tests.h>
#include <mmbits/define_utime_nowomit.h>

int chmod(const char *path, mode_t mode);
int fchmod(int fildes, mode_t mode);
int fchmodat(int fd, const char *path, mode_t mode, int flag);
int fstat(int fildes, struct stat *buf);
int fstatat(int fd, const char *path, struct stat *buf, int flag);
int futimens(int fd, const struct timespec times[2]);
int lstat(const char *path, struct stat *buf);
int mkdir(const char *pathname, mode_t mode);
int mkdirat(int dirfd, const char *pathname, mode_t mode);
int mkfifo(const char *pathname, mode_t mode);
int mkfifoat(int dirfd, const char *pathname, mode_t mode);
int mknod(const char *path, mode_t mode, dev_t dev);
int mknodat(int fd, const char *path, mode_t mode, dev_t dev);
int stat(const char *path, struct stat *buf);
mode_t umask(mode_t cmask);
int utimensat(int dirfd, const char *pathname, const struct timespec times[2], int flags);

#endif // _SYS_STAT_H
