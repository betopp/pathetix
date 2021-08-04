//mmlibc/include/pwd.h
//Password declarations for MMK's libc
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _PWD_H
#define _PWD_H

#include <mmbits/struct_passwd.h>
#include <mmbits/typedef_gid.h>
#include <mmbits/typedef_uid.h>
#include <mmbits/typedef_size.h>

void endpwent(void);
struct passwd *getpwent(void);
struct passwd *getpwnam(const char *name);
int getpwnam_r(const char *name, struct passwd *pwd, char *buf, size_t buflen, struct passwd **result);
struct passwd *getpwuid(uid_t uid);
int getpwuid_r(uid_t uid, struct passwd *pwd, char *buf, size_t buflen, struct passwd **result);
void setpwent(void);

#endif //_PWD_H
