//mmlibc/include/sys/mman.h
//Memory management declarations for MMK's libc.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _SYS_MMAN_H
#define _SYS_MMAN_H

#include <mmbits/define_prot_options.h>
#include <mmbits/define_map_options.h>
#include <mmbits/define_ms_options.h>
#include <mmbits/define_mcl_options.h>

#include <mmbits/define_mapfailed.h>

#include <mmbits/define_madv_options.h>
#include <mmbits/define_typedmem_options.h>

#include <mmbits/typedef_mode.h>
#include <mmbits/typedef_off.h>
#include <mmbits/typedef_size.h>
#include <mmbits/struct_typed_mem_info.h>

int mlock(const void *addr, size_t len);
int mlockall(int flags);
void *mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off);
int mprotect(void *addr, size_t len, int prot);
int msync(void *addr, size_t len, int flags);
int munlock(const void *addr, size_t len);
int munlockall(void);
int munmap(void *addr, size_t len);
int posix_madvise(void *addr, size_t len, int advice);
int posix_mem_offset(const void *addr, size_t len, off_t *off, size_t *contig_len, int *fildes);
int posix_typed_mem_get_info(int fildes, struct posix_typed_mem_info *info);
int posix_typed_mem_open(const char *name, int oflag, int tflag);
int shm_open(const char *name, int oflag, mode_t mode);
int shm_unlink(const char *name);

#endif // _SYS_MMAN_H
