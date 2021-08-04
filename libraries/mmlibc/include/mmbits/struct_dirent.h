//mmlibc/include/mmbits/struct_dirent.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _STRUCT_DIRENT_H
#define _STRUCT_DIRENT_H

#include <mmbits/typedef_ino.h>

struct dirent
{
	ino_t d_ino; //File serial number
	char d_name[]; //Filename string of entry
};

#endif //_STRUCT_DIRENT_H
