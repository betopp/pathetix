//mem.h
//Memory space tracking in kernel
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef MEM_H
#define MEM_H

#include "hal_uspc.h"
#include <sys/types.h>

//Eh just make the info fit in one page
#define MEM_SEG_MAX 120

#define MEM_PROT_R 0x4
#define MEM_PROT_W 0x2
#define MEM_PROT_X 0x1

//Information about a region of memory mapped in a memory space.
typedef struct mem_seg_s
{
	//Range in the address space occupied by this segment
	uintptr_t start; //First address occupied
	uintptr_t end; //First address not occupied
	
	//Permissions userspace should have
	int prot;
	
	//Todo - some architectures may want ephemeral pagetables and need a list of frames here.
	//Currently the frames backing this segment are only referenced in the pagetables.
	
} mem_seg_t;

//Information about a memory space overall.
//Note - not locked! Should belong to a structure with locking.
typedef struct mem_space_s
{
	//Kernel-side tracking of segments in the space.
	//Sorted by address. Non-overlapping.
	mem_seg_t seg_array[MEM_SEG_MAX];
	
	//HAL paging structures for the CPU
	hal_uspc_id_t uspc;
	
} mem_space_t;


//Makes a new empty memory space
mem_space_t *mem_space_new(void);

//Makes a copy of the given memory space.
mem_space_t *mem_space_fork(mem_space_t *old);

//Deletes the given memory space
void mem_space_delete(mem_space_t *mptr);

//Adds an anonymous segment to the given memory space.
//Returns its index on success or a negative error number.
int mem_space_add(mem_space_t *mptr, uintptr_t addr, size_t size, int prot);

//Finds a free region in the memory space for the given size around the given address.
//Returns the address found or a negative error number on failure.
intptr_t mem_space_avail(mem_space_t *mptr, uintptr_t around, size_t size);

//Chops or removes any memory segments overlapping the given range.
int mem_space_clear(mem_space_t *mptr, uintptr_t addr, size_t size);


#endif //MEM_H
