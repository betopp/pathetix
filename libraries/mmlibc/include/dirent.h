//mmlibc/include/dirent.h
//Directory entry structure for MMK's libc.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _DIRENT_H
#define _DIRENT_H

//Opaque type for directory-stream objects
typedef struct _DIR_s DIR;

#include <mmbits/struct_dirent.h>
#include <mmbits/typedef_ino.h>

int alphasort(const struct dirent **a, const struct dirent **b);
int closedir(DIR *dirp);
int dirfd(DIR *dirp);
DIR *fdopendir(int fd);
DIR *opendir(const char *name);
struct dirent *readdir(DIR *dirp);
void rewinddir(DIR *dirp);

int scandir(const char *dirp,
	struct dirent ***namelist, 
	int (*filter)(const struct dirent*), 
	int (*compar)(const struct dirent**, const struct dirent**));
	
void seekdir(DIR *dirp, long loc);
long telldir(DIR *dirp);

//Linux calls this deprecated and I agree. Gone.
//int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result);

#endif //_DIRENT_H
