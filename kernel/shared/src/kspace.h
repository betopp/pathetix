//kspace.h
//Kernel space allocator
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef KSPACE_H
#define KSPACE_H

#include <sys/types.h>
#include <unistd.h>

#include "hal_frame.h"

#define alignof(x) __alignof__(x)


//Allocates pages in kernel-space to satisfy the given size and alignment.
//Backs them with unique frames from the frame allocator.
//Returns the address of the allocation, or NULL on failure.
void *kspace_alloc(size_t bytes, size_t align);

//Frees an allocation previously made with kspace_alloc.
void kspace_free(void *addr, size_t bytes);


//Maps a contiguous range of physical frames in free kernel space.
//Does not perform any frame allocation.
void *kspace_phys_map(hal_frame_id_t paddr, size_t bytes);

//Frees a range of physical space previously mapped with kspace_phys_map.
//Does not perform any frame allocation.
void kspace_phys_unmap(void *vaddr, size_t bytes);


#endif //KSPACE_H
