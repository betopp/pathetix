//mmlibc/include/stdlib.h
//Standard library functions for MMK's libc
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _STDLIB_H
#define _STDLIB_H

#define EXIT_FAILURE (-1)
#define EXIT_SUCCESS (0)
#define RAND_MAX (32767) //For awful POSIX-example rand()

#define MB_CUR_MAX ((size_t)1)

#include <mmbits/define_null.h>
#include <mmbits/typedef_div.h>
#include <mmbits/typedef_size.h>

void _Exit(int status);
void abort(void);
int abs(int j);
int atexit(void (*function)(void));
double atof(const char *nptr);
int atoi(const char *nptr);
long atol(const char *nptr);
long long atoll(const char *nptr);
void *bsearch(const void *key, const void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));
void *calloc(size_t nmemb, size_t size);
div_t div(int numerator, int denominator);
void exit(int status);
void free(void *ptr);
char *getenv(const char *name);
int getsubopt(char **optionp, char *const *tokens, char **valuep);
long labs(long j);
ldiv_t ldiv(long numerator, long denominator);
long long llabs(long long j);
lldiv_t lldiv(long long numerator, long long denominator);
void *malloc(size_t size);
int mblen(const char *s, size_t n);
char *mkdtemp(char *templ);
char *mktemp(char *templ);
int mkstemp(char *templ);
int posix_memalign(void **memptr, size_t alignment, size_t size);
void qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));
int rand(void);
void *realloc(void *ptr, size_t size);
int setenv(const char *name, const char *value, int overwrite);
void srand(unsigned seed);
double strtod(const char * nptr, char **endptr);
float strtof(const char *nptr, char **endptr);
long strtol(const char *nptr, char **endptr, int base);
long double strtold(const char *nptr, char **endptr);
long long strtoll(const char *nptr, char **endptr, int base);
unsigned long strtoul(const char *nptr, char **endptr, int base);
unsigned long long strtoull(const char *nptr, char **endptr, int base);
int system(const char *command);
int unsetenv(const char *name);


#endif //_STDLIB_H
