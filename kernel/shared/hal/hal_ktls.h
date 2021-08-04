//hal_ktls.h
//Per-arch kernel thread pointer
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef HAL_KTLS_H
#define HAL_KTLS_H

//Sets the thread pointer for the current context.
void hal_ktls_set(void *ptr);

//Returns the thread pointer for the current context.
void *hal_ktls_get(void);

#endif //HAL_KTLS_H
