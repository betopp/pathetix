//passwd.c
//Password database stubs in libc
//Bryan E. Topp <betopp@betopp.com> 2021

#include <pwd.h>
#include <stddef.h>
#include <string.h>

struct passwd _passwd_buf;

struct passwd *getpwnam(const char *login)
{
	//Stub for single-user for now
	if(login == NULL || strcmp(login, "root") != 0)
		return NULL;
	
	static char rootname[5] = { 'r', 'o', 'o', 't', '\0' };
	_passwd_buf.pw_name = rootname;
	_passwd_buf.pw_uid = 0;
	_passwd_buf.pw_gid = 0;
	static char rootdir[2] = { '/', '\0' };
	_passwd_buf.pw_dir = rootdir;
	static char rootshell[10] = { '/', 'b', 'i', 'n', '/', 'o', 'k', 's', 'h', '\0' };
	_passwd_buf.pw_shell = rootshell;
	
	return &_passwd_buf;
}
