//hal_uspc.h
//Architecture-specific userspace management
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef HAL_USPC_H
#define HAL_USPC_H

#include "hal_frame.h"

//Type identifying a userspace.
typedef uintptr_t hal_uspc_id_t;
#define HAL_USPC_ID_INVALID ((hal_uspc_id_t)(0))

//Creates a new userspace, empty except for kernel mapping as necessary.
hal_uspc_id_t hal_uspc_new(void);

//Deletes the given userspace, freeing all paging structures (but not any referenced frames!)
void hal_uspc_delete(hal_uspc_id_t id);

//Sets the mapping of a page in the given userspace.
//Pass frame 0 to unmap the page.
//May fail if there's not enough frames left for paging structures.
//Returns 0 on success or -1 on failure.
int hal_uspc_set(hal_uspc_id_t id, uintptr_t vaddr, hal_frame_id_t frame);

//Returns the mapping of a page in the given userspace, or 0 if not mapped.
hal_frame_id_t hal_uspc_get(hal_uspc_id_t id, uintptr_t vaddr);

//Activates the given userspace.
//If ID is HAL_USPC_ID_INVALID, switches back to kernel-space only.
void hal_uspc_activate(hal_uspc_id_t id);

//Returns the current active userspace.
hal_uspc_id_t hal_uspc_current(void);

//Returns the bounds of addresses usable for user-spaces.
void hal_uspc_bound(uintptr_t *start_out, uintptr_t *end_out);

#endif //HAL_USPC_H
