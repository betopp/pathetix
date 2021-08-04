//mmlibc/include/strings.h
//Further string definitions for MMK's libc
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _STRINGS_H
#define _STRINGS_H

#include <mmbits/typedef_size.h>
#include <mmbits/typedef_locale.h>

int ffs(int i);
int strcasecmp(const char *s1, const char *s2);
int strcasecmp_l(const char *s1, const char *s2, locale_t locale);
int strncasecmp(const char *s1, const char *s2, size_t n);
int strncasecmp_l(const char *s1, const char *s2, size_t n, locale_t locale);

#endif //_STRINGS_H

