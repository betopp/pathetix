//dirs.h
//Directory searching for C compiler
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef DIRS_H
#define DIRS_H

#include <stdio.h>

//Search path types
//Included files are searched in "user" includes if double-quoted, "system" includes if angle-bracketed.
#define DIRS_USR 0
#define DIRS_SYS 1

//Adds a search directory
void dirs_add(int dirs, const char *path);

//Tries to open a file in the given set of search directories
FILE *dirs_find(int dirs, const char *file);

#endif //DIRS_H
