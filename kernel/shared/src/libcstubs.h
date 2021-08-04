//libcstubs.h
//libc-like functions in kernel
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef LIBCSTUBS_H
#define LIBCSTUBS_H

#include <sys/types.h>

int memcmp(const void *s1, const void *s2, size_t n);
void *memset(void *s, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
size_t strlen(const char *s);
char *strncpy(char *dest, const char *src, size_t n);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strchr(const char *s, int c);

#endif //LIBCSTUBS_H
