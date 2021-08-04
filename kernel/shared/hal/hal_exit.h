//hal_exit.h
//Kernel exit context buffers
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef HAL_EXIT_H
#define HAL_EXIT_H

//Words in the exit buffer that the kernel expects in a certain place
#define HAL_EXIT_IDX_SZ 0 //Size of the buffer
#define HAL_EXIT_IDX_PC 1 //User return address
#define HAL_EXIT_IDX_SP 2 //User stack pointer
#define HAL_EXIT_IDX_RV 3 //User return-value

//Buffer big enough to hold any possible return context.
typedef struct hal_exit_s
{
	//Todo - should be arch-dependent.
	//AMD64 is MASSIVE due to crap like SSE.
	uint64_t vals[128];
} hal_exit_t;

//Initially exits the kernel to a specified user entry point, to be re-entered at the given stack pointer.
void hal_exit_fresh(uintptr_t u_pc, void *k_sp);

//Exits the kernel to userland using the given existing enter/exit buffer, to be re-entered at the given stack pointer.
void hal_exit_resume(hal_exit_t *e, void *k_sp);

#endif //HAL_EXIT_H

