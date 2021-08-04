//kassert.h
//Kernel assertion macro
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef KASSERT_H
#define KASSERT_H

#include "hal_panic.h"

//Macro runaround to get line number as a string
#define KASSERT_STR1(ll) #ll
#define KASSERT_STR2(ll) KASSERT_STR1(ll)

//Called when an assertion fails. Attempts to print a message and panics.
void kassert_failed(const char *file, const char *line, const char *func, const char *cond);

//Stops the machine with an error if the condition is false.
#define KASSERT(cond) do{if(!(cond)){kassert_failed(__FILE__, KASSERT_STR2(__LINE__), __FUNCTION__, #cond);}}while(0)

#endif //KASSERT_H
