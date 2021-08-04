//alloc.h
//Shim for malloc that aborts on failure to allocate RAM
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef ALLOC_H
#define ALLOC_H

#include <sys/types.h>

//Allocates and zeroes memory. Aborts on failure.
void *alloc_mandatory(size_t size);

//Reallocates the given buffer for a new size. Aborts on failure.
void *realloc_mandatory(void *ptr, size_t size);

//Duplicates the given string into a newly allocated buffer. Aborts on failure.
char *strdup_mandatory(const char *str);

#endif //ALLOC_H
