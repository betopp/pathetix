//mmlibc/include/mmbits/define_modes.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _DEFINE_MODES_H
#define _DEFINE_MODES_H

#define S_IFMT   ((mode_t)0770000)

#define S_IFREG  ((mode_t)0010000)
#define S_IFDIR  ((mode_t)0020000)
#define S_IFBLK  ((mode_t)0030000)
#define S_IFCHR  ((mode_t)0040000)
#define S_IFLNK  ((mode_t)0050000)
#define S_IFSOCK ((mode_t)0060000)
#define S_IFIFO  ((mode_t)0070000)

#endif //_DEFINE_MODES_H
