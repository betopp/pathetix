//mmlibc/include/mmbits/define_mode_tests.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _DEFINE_MODE_TESTS_H
#define _DEFINE_MODE_TESTS_H


#define S_ISBLK(m) ( (m & S_IFMT) == S_IFBLK )
#define S_ISCHR(m) ( (m & S_IFMT) == S_IFCHR )
#define S_ISDIR(m) ( (m & S_IFMT) == S_IFDIR )
#define S_ISFIFO(m) ( (m & S_IFMT) == S_IFIFO )
#define S_ISREG(m) ( (m & S_IFMT) == S_IFREG )
#define S_ISLNK(m) ( (m & S_IFMT) == S_IFLNK )
#define S_ISSOCK(m) ( (m & S_IFMT) == S_IFSOCK )

#define S_TYPEISMQ(buf) (0)
#define S_TYPEISSEM(buf) (0)
#define S_TYPEISSHM(buf) (0)
#define S_TYPEISTMO(buf) (0)


#endif //_DEFINE_MODE_TESTS_H
