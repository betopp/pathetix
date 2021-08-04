//mmlibc/include/grp.h
//Group-related definitions for MMK's libc
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _GRP_H
#define _GRP_H

#include <mmbits/struct_group.h>
#include <mmbits/typedef_gid.h>
#include <mmbits/typedef_size.h>

void endgrent(void);
struct group *getgrent(void);
struct group *getgrgid(gid_t gid);
int getgrgid_r(gid_t gid, struct group *grp, char *buf, size_t buflen, struct group **result);
struct group *getgrnam(const char *name);
int getgrnam_r(const char *name, struct group *grp, char *buf, size_t buflen, struct group **result);
void setgrent(void);

#endif //_GRP_H

