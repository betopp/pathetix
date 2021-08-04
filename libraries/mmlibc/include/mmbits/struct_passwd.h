//mmlibc/include/mmbits/struct_passwd.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _STRUCT_PASSWD_H
#define _STRUCT_PASSWD_H

#include <mmbits/typedef_uid.h>
#include <mmbits/typedef_gid.h>

struct passwd
{
	char *pw_name;  //User's login name
	uid_t pw_uid;   //Numerical user ID
	gid_t pw_gid;   //Numerical group ID
	char *pw_dir;   //Initial working directory
	char *pw_shell; //Program to run as shell
};

#endif //_STRUCT_PASSWD_H

