//mmlibc/include/mmbits/define_mode_flags.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _DEFINE_MODE_FLAGS_H
#define _DEFINE_MODE_FLAGS_H


#include <mmbits/typedef_mode.h>

#define S_ISUID ((mode_t)04000)
#define S_ISGID ((mode_t)02000)
#define S_ISVTX ((mode_t)01000)
#define S_IRWXU ((mode_t)00700)
#define S_IRUSR ((mode_t)00400)
#define S_IWUSR ((mode_t)00200)
#define S_IXUSR ((mode_t)00100)
#define S_IRWXG ((mode_t)00070)
#define S_IRGRP ((mode_t)00040)
#define S_IWGRP ((mode_t)00020)
#define S_IXGRP ((mode_t)00010)
#define S_IRWXO ((mode_t)00007)
#define S_IROTH ((mode_t)00004)
#define S_IWOTH ((mode_t)00002)
#define S_IXOTH ((mode_t)00001)

	
#endif //_DEFINE_MODE_FLAGS_H
