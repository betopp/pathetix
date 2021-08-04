//mmlibc/include/string.h
//String definitions for MMK's libc
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _STRING_H
#define _STRING_H

#include <mmbits/define_null.h>
#include <mmbits/typedef_size.h>
#include <mmbits/typedef_locale.h>

void *memccpy(void *dest, const void *src, int c, size_t n);
void *memchr(const void *s, int c, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
char *stpcpy(char *dest, const char *src);
char *stpncpy(char *dest, const char *src, size_t n);
char *strcat(char *dest, const char *src);
char *strchr(const char *s, int c);
int strcmp(const char *s1, const char *s2);
int strcoll(const char *s1, const char *s2);
int strcoll_l(const char *s1, const char *s2, locale_t locale);
char *strcpy(char *dest, const char *src);
size_t strcspn(const char *s, const char *reject);
char *strdup(const char *s);
char *strerror(int errnum);
char *strerror_l(int errnum, locale_t locale);
int strerror_r(int errnum, char *buf, size_t buflen);
size_t strlen(const char *s);
char *strncat(char *dest, const char *src, size_t n);
int strncmp(const char *s1, const char *s2, size_t n);
char *strncpy(char *dest, const char *src, size_t n);
char *strndup(const char *s, size_t n);
size_t strnlen(const char *s, size_t maxlen);
char *strpbrk(const char *s, const char *accept);
char *strrchr(const char *s, int c);
char *strsignal(int sig);
size_t strspn(const char *s, const char *accept);
char *strstr(const char *haystack, const char *needle);
char *strtok(char *str, const char *delim);
char *strtok_r(char *str, const char *delim, char **saveptr);
size_t strxfrm(char *dest, const char *src, size_t n);
size_t strxfrm_l(char *dest, const char *src, size_t n, locale_t locale);

#endif //_STRING_H

