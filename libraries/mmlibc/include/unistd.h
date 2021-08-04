//mmlibc/include/unistd.h
//Miscellaneous definitions for MMK's libc
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _UNISTD_H
#define _UNISTD_H

#define _POSIX_VERSION 200809L
#define _XOPEN_VERSION 700

#include <mmbits/define_null.h>
#include <mmbits/define_access_options.h>
#include <mmbits/define_confstr_options.h>
#include <mmbits/define_lockf_options.h>
#include <mmbits/define_pathconf_options.h>
#include <mmbits/define_sysconf_options.h>

#include <mmbits/typedef_size.h>
#include <mmbits/typedef_ssize.h>
#include <mmbits/typedef_uid.h>
#include <mmbits/typedef_gid.h>
#include <mmbits/typedef_off.h>
#include <mmbits/typedef_pid.h>
#include <mmbits/typedef_useconds.h>

#include <mmbits/typedef_intptr.h>

#include <mmbits/define_whence_options.h>

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2


#define _POSIX_VDISABLE 0

int access(const char *path, int amode);
unsigned alarm(unsigned seconds);
int chdir(const char *path);
int chown(const char *pathname, uid_t owner, gid_t group);
int close(int fd);
size_t confstr(int name, char *buf, size_t len);
char *crypt(const char *key, const char *salt);
int dup(int fildes);
int dup2(int fildes, int fildes2);
void _exit(int status);
void encrypt(char block[64], int edflag);
int execl(const char *path, const char *arg0, ...);
int execle(const char *path, const char *arg0, ...);
int execlp(const char *file, const char *arg0, ...);
int execv(const char *path, char *const argv[]);
int execve(const char *path, char *const argv[], char *const envp[]);
int execvp(const char *file, char *const argv[]);
int faccessat(int fd, const char *path, int amode, int flag);
int fchdir(int fildes);
int fchown(int fildes, uid_t owner, gid_t group);
int fchownat(int fd, const char *path, uid_t owner, gid_t group, int flag);
int fdatasync(int fildes);
int fexecve(int fd, char *const argv[], char *const envp[]);
int funlinkat(int dfd, const char *path, int fd, int flag); //like FreeBSD
pid_t fork(void);
long fpathconf(int fd, int name);
int fsync(int fildes);
int ftruncate(int fildes, off_t length);
char *getcwd(char *buf, size_t size);
gid_t getegid(void);
uid_t geteuid(void);
gid_t getgid(void);
int getgroups(int gidsetsize, gid_t grouplist[]);
long gethostid(void);
int gethostname(char *name, size_t namelen);
char *getlogin(void);
int getlogin_r(char *buf, size_t bufsize);
int getopt(int argc, char * const argv[], const char *optstring);
pid_t getpgid(pid_t pid);
pid_t getpgrp(void);
pid_t getpid(void);
pid_t getppid(void);
pid_t getsid(pid_t pid);
uid_t getuid(void);
int isatty(int fd);
int lchown(const char *path, uid_t owner, gid_t group);
int link(const char *path1, const char *path2);
int linkat(int fd1, const char *path1, int fd2, const char *path2, int flag);
int lockf(int fd, int cmd, off_t len);
off_t lseek(int fd, off_t offset, int whence);
int nice(int incr);
long pathconf(const char *path, int name);
int pause(void);
int pipe(int fd[2]);
ssize_t pread(int, void *, size_t, off_t);
ssize_t pwrite(int, const void *, size_t, off_t);
ssize_t read(int fd, void *buf, size_t count);
ssize_t readlink(const char *, char *, size_t);
ssize_t readlinkat(int, const char *, char *, size_t);
int rmdir(const char *path);
int setegid(gid_t gid);
int seteuid(uid_t uid);
int setgid(gid_t gid);
int setpgid(pid_t pid, pid_t pgid);
pid_t setpgrp(void);
int setregid(gid_t rgid, gid_t egid);
int setreuid(uid_t ruid, uid_t euid);
pid_t setsid(void);
int setuid(uid_t uid);
unsigned sleep(unsigned seconds);
void swab(const void *from, void *to, ssize_t n);
int symlink(const char *path1, const char *path2);
int symlinkat(const char *path1, int fd, const char *path2);
void sync(void);
long sysconf(int name);
pid_t tcgetpgrp(int fd);
int tcsetpgrp(int fd, pid_t pgrp);
int truncate(const char *path, off_t length);
char *ttyname(int fd);
int ttyname_r(int fd, char *buf, size_t buflen);
int unlink(const char *path);
int unlinkat(int fd, const char *path, int flag);
int usleep(useconds_t useconds);
ssize_t write(int fd, const void *buf, size_t count);

//Non-POSIX additional functions that everyone seems to expect
int setgroups(size_t gidsetsize, const gid_t *grouplist);

#endif //_UNISTD_H
