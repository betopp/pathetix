//hal_panic.h
//Fatal error reporting
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef HAL_PANIC_H
#define HAL_PANIC_H

//Stops the machine and prints the error if possible.
void hal_panic(const char *str) __attribute__((noreturn));

#endif //HAL_PANIC_H
