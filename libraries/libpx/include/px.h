//px.h
//Pathetix system call interface
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef PX_H
#define PX_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

//Opens an existing file, in a directory already opened, with only permission to stat it.
//Passing a negative number for "at" causes the search in the present directory.
//Passing a name of "/" causes the root directory to be opened.
//Returns a number identifying the new file descriptor, or a negative error number.
int px_fd_find(int at, const char *name);

//Changes and returns the access mode of a file descriptor.
//set and clr describe which access bits to set and then to clear, respectively.
//Returns the new set of access bits, or a negative error number.
#define PX_FD_ACCESS_R 4 //Can read
#define PX_FD_ACCESS_W 2 //Can write
#define PX_FD_ACCESS_X 1 //Can exec or search directory
int px_fd_access(int fd, int set, int clr);

//Changes and returns the flags associated with a file descriptor number, LOCAL TO THAT REFERENCE NUMBER.
//set and clr describe which bits to set and then to clear, respectively.
//Returns the new set of flag bits, or a negative error number.
#define PX_FD_FLAG_KEEPEXEC 1 //Reference remains across exec; opposite of CLOEXEC / O_CLOEXEC.
int px_fd_flag(int fd, int set, int clr);

//Uniform format of directory entries returned when reading a directory.
#define PX_FD_DIRENT_NAME_BUFLEN 256
typedef struct px_fd_dirent_s
{
	uint64_t next; //Absolute offset where the next directory entry can be read.
	uint64_t ino; //Inode of the file referenced
	uint64_t dummy1; //Reserved for forward-compatibility
	uint64_t dummy2; //Reserved for forward-compatibility
	uint64_t dummy3; //Reserved for forward-compatibility
	uint64_t dummy4; //Reserved for forward-compatibility
	uint64_t dummy5; //Reserved for forward-compatibility
	uint64_t dummy6; //Reserved for forward-compatibility
	char name[PX_FD_DIRENT_NAME_BUFLEN]; //Name of the file in the directory
} px_fd_dirent_t;

//Reads from a file. The file must be open for reading.
//Returns the number of bytes read or a negative error number.
ssize_t px_fd_read(int fd, void *buf, size_t len);

//Writes to a file. The file must be open for writing.
//Returns the number of bytes written or a negative error number.
ssize_t px_fd_write(int fd, const void *buf, size_t len);

//Seeks, changing the file pointer in an open file. The file must be open for reading or writing.
//Returns the new location of the file pointer in the file.
off_t px_fd_seek(int fd, off_t off, int whence);

//Changes the size of a file. Returns 0 on success or a negative error number.
int px_fd_trunc(int fd, off_t size);

//Creates a new file. Returns a file descriptor to it or a negative error number.
//If the given mode specifies a directory, a directory is created and "." and ".." entries are made.
//If the given mode specifies a block or character special, then "spec" determines the device number.
int px_fd_create(int at, const char *name, mode_t mode, uint64_t spec);

//Removes a link to a file. Removes the file if the last link was removed and no process has it open.
//If onlyfd is nonnegative, will only unlink the file if it is the one referenced by the given file descriptor.
//If rmdir is 1, only removes (empty) directories. If rmdir is 0, only removes non-directories.
//Returns 0 on success or a negative error number.
int px_fd_unlink(int at, const char *name, int onlyfd, int rmdir);

//Status information returned about a file
typedef struct px_fd_stat_s
{
	uint64_t dev;
	uint64_t ino;
	uint64_t size;
	uint64_t mode;
	uint64_t spec;
	uint64_t dummy1;
	uint64_t dummy2;
	uint64_t dummy3;
	uint64_t dummy4;
	uint64_t dummy5;
	uint64_t dummy6;
	uint64_t dummy7;
	uint64_t dummy8;
	uint64_t dummy9;
	uint64_t dummya;
	uint64_t dummyb;
} px_fd_stat_t;

//Retrieves status information about a file, writing into the given structure.
//Returns the size of structure filled or a negative error number.
ssize_t px_fd_stat(int fd, px_fd_stat_t *buf, size_t len);

//Closes the given file descriptor.
//Returns 0 on success or a negative error number.
int px_fd_close(int fd);

//Executes the program represented by the given file descriptor.
//The file must be open for execution.
//Returns a negative error number, or doesn't return on success.
int px_fd_exec(int fd, char * const * argv, char * const * envp);

//Makes another reference to an existing file descriptor.
//newmin specifies the minimum file descriptor number to use for the new reference.
//overwrite specifies whether to search for a free number starting at newmin, or just overwrite the one at newmin.
//Returns the new file descriptor number, or a negative error number.
int px_fd_dup(int oldfd, int newmin, bool overwrite);

//Performs device-specific or RPC-oriented operations on a file descriptor.
//Returns negative values on failure and nonnegative values otherwise.
//Todo - for now just have some hardcoded values for the shell to operate the console device.
#define PX_FD_IOCTL_ISATTY 1
#define PX_FD_IOCTL_TTYNAME 2
#define PX_FD_IOCTL_GETATTR 3
#define PX_FD_IOCTL_SETATTR 4
#define PX_FD_IOCTL_GETPGRP 5
#define PX_FD_IOCTL_SETPGRP 6
#define PX_FD_IOCTL_GETGFXM 7 //Get graphics mode
#define PX_FD_IOCTL_SETGFXM 8 //Set graphics mode
int px_fd_ioctl(int fd, uint64_t request, void *ptr, size_t len);

//Sets the working directory of the calling process to that described by the given file descriptor.
//Another reference is made to the file descriptor (as in dup), and used whenever the working directory is called for.
//Returns 0 on success or a negative error number.
int px_chdir(int fd);

//Terminates the calling process with the given 8-bit exit code.
//Optionally reports that the termination was due to the given signal if nonzero.
void px_exit(int code, int signal);

//Returns the process ID of the calling process.
pid_t px_getpid(void);

//Returns the process ID of the parent of the calling process.
pid_t px_getppid(void);

//Returns the process group ID of the given process, or the calling process if pid==0.
pid_t px_getpgid(pid_t pid);

//Sets the process group ID of the given process, or the calling process if pid==0.
//The process group ID is set to that of the calling process if pgrp==0.
//Returns 0 on success or a negative error number.
int px_setpgid(pid_t pid, pid_t pgrp);

//Resource limit specification
typedef struct px_rlimit_s
{
	uint64_t cur;
	uint64_t max;
} px_rlimit_t;

//Sets a resource limit for the calling process.
int px_setrlimit(int resource, const px_rlimit_t *ptr, size_t len);

//Returns a resource limit for the calling process.
int px_getrlimit(int resource, px_rlimit_t *ptr, size_t len);

//Resource usage reported
typedef struct px_rusage_s
{
	int64_t utime_sec;
	int64_t utime_usec;
	int64_t stime_sec;
	int64_t stime_usec;
} px_rusage_t;

//Returns resource usage for the current thread, current process, or process and all children.
#define PX_RUSAGE_THREAD 1
#define PX_RUSAGE_PROCESS 0
#define PX_RUSAGE_CHILDREN -1
int px_rusage(int who, px_rusage_t *ptr, size_t len);


//Changes the signal mask of the calling thread; returns the previous value of the signal mask or a negative error number.
//This implies that a maximum of 63 signals are supported.
int64_t px_sigmask(int how, int64_t val);

//Temporarily sets the signal mask to the given mask and waits for any signal to occur.
//Returns -EINTR when successful.
int px_sigsuspend(int64_t tempmask);

//Sends a signal to the given thread, process, or process group (because "kill()" is a misnomer).
int px_sigsend(idtype_t to_type, int64_t to_id, int sig);

//Information provided about a signal handler.
typedef struct px_siginfo_s
{
	int signum; //Signal number that occurred.
	int64_t sigmask; //Signal mask at time signal occurred.
	int sender; //Sender if signal sent from another process.
	uintptr_t instruction; //Address of program counter of faulting instruction, if any
	uintptr_t referenced; //Address of invalid access if any
	
} px_siginfo_t;

//If running in a signal handler, retrieves information about the signal handler. Fills the given buffer.
//Returns the amount of data filled on success or a negative error number.
ssize_t px_siginfo(px_siginfo_t *out_ptr, size_t out_len);

//If running in a signal handler, returns from the signal handler to the given context.
//Doesn't return on success. Returns a negative error number on failure.
int px_sigexit(void);


//Reads the real-time clock of the system.
//Value returned is absolute microseconds elapsed since the GPS epoch.
//No particular precision is implied.
//Returns a negative error number on failure (primarily, if the RTC was never set).
int64_t px_getrtc(void);

//Sets the real-time clock of the system.
//Value should be absolute microseconds elapsed since the GPS epoch.
//Returns 0 on success or a negative error number.
int px_setrtc(int64_t val);

//Creates a copy of the calling process.
//The new process retains references to all file descriptors at their same numbers.
//The new process starts with a copy of all memory from the calling process.
//The new process starts with only one thread, executing at the given address.
//Returns the PID of the child or a negative error number.
//The syscall itself does not return on the child - libc uses setjmp/longjmp trickery to do that.
pid_t px_fork(uintptr_t child_entry_pc);

//Deschedules the calling thread for at least the specified amount of nanoseconds.
//No amount of precision is implied.
//Returns 0 on success or a negative error number (notably, -EINTR).
int px_nanosleep(int64_t ns);

//Status information returned about a child process by px_wait.
typedef struct px_wait_s
{
	int64_t pid; //ID of child process
	int exitst; //Exit status of the child - what they passed to exit
	int waitst; //Wait status of the child - what about the process changed (WEXITED, etc)
	
} px_wait_t;

//Waits for status information about a child process.
//Returns the size of structure filled or a negative error number.
ssize_t px_wait(idtype_t id_type, int64_t id, int options, px_wait_t *ptr, size_t len);

//Sets or gets priority for the given user/process/pgrp.
//Returns the old priority or a negative error number.
//Pass -1 as ID to mean "this".
//The kernel uses priority 0-99. Pass -1 to prval to query without changing.
int px_priority(idtype_t id_type, int64_t id, int prval);

//Sets a timer. The timer starts counting down from the given value.
//If interval is nonzero, the timer will be reloaded automatically with interval on expiration.
//Returns the previous value of the timer on success or a negative error number.
int64_t px_timer_set(timer_t id, int flags, int64_t value_ns, int64_t interval_ns);

//Returns the current value of a timer.
int64_t px_timer_get(timer_t id);

//Protection options for memory
#define PX_MEM_R 4
#define PX_MEM_W 2
#define PX_MEM_X 1

//Finds a free region of at least the given size, near the given address, in the calling process's memory map.
//Returns the address of the region or a negative error number.
intptr_t px_mem_avail(uintptr_t around, size_t size);

//Adds new anonymous private memory to the calling process's memory map.
//Fails if any of the given region is already in use.
//Returns 0 on success or a negative error number.
int px_mem_anon(uintptr_t start, size_t size, int prot);


#endif //PX_H
