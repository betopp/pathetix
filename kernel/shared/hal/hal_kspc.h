//hal_kspc.h
//Kernel-space management
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef HAL_KSPC_H
#define HAL_KSPC_H

#include "hal_frame.h"

//Kernel-space, for our purposes, is a region of memory that does not change when user-space changes.

//Returns the bounds of kernel-space addresses dynamically usable by the kernel.
//This excludes the kernel as linked and excludes any addresses the HAL needs to use.
void hal_kspc_bound(uintptr_t *start_out, uintptr_t *end_out);

//Sets the frame backing the given kernel-space page.
//Unmaps the given page if 0 is passed as the frame.
//Returns 0 on success or -1 on failure.
//This may fail if there's no frames left for paging structures!
int hal_kspc_set(uintptr_t vaddr, hal_frame_id_t frame);

//Returns the frame backing the given kernel-space page.
//Returns 0 if there's no frame backing it.
hal_frame_id_t hal_kspc_get(uintptr_t vaddr);


#endif //HAL_KSPC_H
