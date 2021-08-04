//mmlibc/include/mmbits/struct_group.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _STRUCT_GROUP_H
#define _STRUCT_GROUP_H

#include <mmbits/typedef_gid.h>

struct group
{
	char *gr_name; //The name of the group
	gid_t gr_gid; //Numerical ID of group
	char **gr_mem; //Pointer to null-terminated array of pointers to member names.
};

#endif //_STRUCT_GROUP_H
