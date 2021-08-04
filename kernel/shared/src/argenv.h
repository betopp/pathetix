//argenv.h
//Argument and environment passing in kernel
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef ARGENV_H
#define ARGENV_H

#include "mem.h"

//Packs the data from the argv/envp parameters into the given memory space.
int argenv_load(mem_space_t *mem, char * const * argv, char * const * envp);

#endif //ARGENV_H

