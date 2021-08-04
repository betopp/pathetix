//hal_ctx.h
//Kernel context switching
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef HAL_CTX_H
#define HAL_CTX_H

#include <sys/types.h>
#include <stdint.h>

//Storage for kernel CPU context.
//Note that this only has to save registers that are preserved across function calls.
//Todo - make this arch-dependent somehow
typedef struct hal_ctx_s { uint64_t regs[16]; } hal_ctx_t;

//Returns the size actually needed to store a CPU context.
size_t hal_ctx_size(void);

//Initializes a CPU context, with the given code/stack/KTLS pointers.
void hal_ctx_reset(void *dst, void (*entry)(void), void *stack_top, void *ktls);

//Switches contexts - saves the old context to one buffer and loads a new context from another.
//Note that this only preserves registers that are normally preserved across a function call.
//In other words, this is for switching between contexts in kernel mode.
//For saving/loading user contexts on preemption, we'll define a different function.
//This switches the current user-space (i.e. the page-directory base) and whether interrupts are enabled.
void hal_ctx_switch(void *save, const void *load);

#endif //HAL_CTX_H
