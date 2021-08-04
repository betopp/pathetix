//ramfs.h
//In-memory root filesystem
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef RAMFS_H
#define RAMFS_H

#include <sys/types.h>
#include "fd.h"

id_t    ramfs_create(fd_t *fd, const char *name, mode_t mode, uint64_t spec);
id_t    ramfs_find  (fd_t *fd, const char *name);
ssize_t ramfs_read  (fd_t *fd, void *buf, size_t len);
ssize_t ramfs_write (fd_t *fd, const void *buf, size_t len);
ssize_t ramfs_stat  (fd_t *fd, px_fd_stat_t *buf, size_t len);
int     ramfs_trunc (fd_t *fd, off_t size);
int     ramfs_unlink(fd_t *fd, const char *name, ino_t only_ino, int rmdir);
void    ramfs_close (fd_t *fd);
int     ramfs_access(fd_t *fd, int set, int clr);

#endif //RAMFS_H
