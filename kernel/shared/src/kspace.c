//kspace.c
//Kernel space allocator
//Bryan E. Topp <betopp@betopp.com> 2021

#include "kspace.h"
#include "kassert.h"
#include "hal_frame.h"
#include "hal_spl.h"
#include "hal_kspc.h"

//Simple round-robin allocator for kernel space.
static uintptr_t kspace_next;

//Spinlock protecting the kernel allocator
static hal_spl_t kspace_spl;

//Finds an unused region in kernel space enough to hold the given number of bytes with the given alignment, and guard pages on each end.
//Returns 0 if none was found, or the address after the beginning guard page if so.
static uintptr_t kspace_findfree(size_t size, size_t align)
{
	KASSERT(size >= 0);
	KASSERT(align >= 0);
	
	size_t pagesize = hal_frame_size();
	KASSERT( (kspace_next % pagesize) == 0 );
	KASSERT( (size % pagesize) == 0 );
	if(align < pagesize)
	{
		align = pagesize; //Can't allocate less aligned than this anyway
	}
	
	uintptr_t kspace_start;
	uintptr_t kspace_end;
	hal_kspc_bound(&kspace_start, &kspace_end);
	
	//Linear search through kernel-space for a free region of the given size, plus guard pages.
	//Use global kspace_next as iterator, so we go round-robin as we perform allocations.
	//In each operation, search up to the entire size of usable kernel-space.
	uintptr_t contiguous_size = 0;
	uintptr_t contiguous_start = 0;
	uintptr_t contiguous_needed = size + (2 * pagesize);
	for(uintptr_t searched = 0; searched < kspace_end - kspace_start; searched += pagesize)
	{		
		if(kspace_next >= kspace_end || kspace_next < kspace_start)
		{
			//Hit end of kernel space. Wrap back around and search from the beginning.
			contiguous_size = 0;
			contiguous_start = 0;
			kspace_next = kspace_start;
		}
		
		if(hal_kspc_get(kspace_next) != HAL_FRAME_ID_INVALID)
		{
			//This page of kernel-space is already allocated.
			contiguous_size = 0;
			contiguous_start = 0;
			kspace_next += pagesize;
			continue;
		}
		
		//This is a valid, free page of kernel space.
		
		//If it's the first free page after allocated pages, note where the region starts.
		if(contiguous_size == 0)
		{
			//Only allow starting on the requested alignment.
			//Note that the relevant alignment starts after the guard page.
			if( ((kspace_next + pagesize) % align) != 0 )
			{
				//Bad alignment, can't start kere
				kspace_next += pagesize;
				continue;
			}
			else
			{
				contiguous_start = kspace_next;
			}
		}
		contiguous_size += pagesize;
		kspace_next += pagesize;
		
		//See if we've found enough space for our allocation
		if(contiguous_size >= contiguous_needed)
		{
			//Success!
			break;
		}
	}
	
	//If we couldn't find enough space, bail out.
	if(contiguous_size < contiguous_needed)
	{
		return 0;
	}
	
	//Otherwise, return the space found, past the guard page
	KASSERT( ((contiguous_start + pagesize) % align) == 0 );
	KASSERT( size + pagesize + pagesize <= contiguous_size );
	return contiguous_start + pagesize;
}

void *kspace_alloc(size_t size, size_t align)
{
	//Handle size=0 case trivially
	if(size == 0)
	{
		return NULL;
	}
	
	if(align <= 0)
	{
		align = 1;
	}
		
	//Lock kernel-space while allocating
	hal_spl_lock(&kspace_spl);
	
	//Figure out how many pages we'll need to allocate to cover this region
	size_t pagesize = hal_frame_size();
	size_t pages_needed = (size + (pagesize - 1)) / pagesize;
	
	//Find a free area of kernel-space containing that number of pages, plus a guard page on each end.
	size_t contiguous_start = kspace_findfree(pages_needed * pagesize, align);
	if(contiguous_start == 0)
	{
		//Failed to find enough contiguous free virtual space.
		hal_spl_unlock(&kspace_spl);
		return NULL;
	}
	
	//Okay, we know where we'll put the new allocation in kernel space.
	//See if we can get enough physical frames to back it.
	uintptr_t alloc_start = contiguous_start;
	uintptr_t alloc_size = pages_needed * pagesize;
	for(uintptr_t frame_iter = alloc_start; frame_iter < alloc_start + alloc_size; frame_iter += pagesize)
	{
		hal_frame_id_t new_frame = hal_frame_alloc();
		if(new_frame == HAL_FRAME_ID_INVALID)
		{
			//Ran out of physical frames. Back up and release any that we did allocate.
			while(frame_iter > alloc_start)
			{
				frame_iter -= pagesize;
				
				hal_frame_id_t backout_frame = hal_kspc_get(frame_iter);
				KASSERT(backout_frame != HAL_FRAME_ID_INVALID);
				
				hal_kspc_set(frame_iter, HAL_FRAME_ID_INVALID);
				hal_frame_free(backout_frame);
			}
			
			hal_spl_unlock(&kspace_spl);
			return NULL;
		}
		
		//Got a frame to back the allocation. Put it in place.
		KASSERT(hal_kspc_get(frame_iter) == HAL_FRAME_ID_INVALID);
		hal_kspc_set(frame_iter, new_frame);
	}
	
	//Success! Return the beginning of the allocation.
	hal_spl_unlock(&kspace_spl);
	return (void*)alloc_start;
}

void kspace_free(void *ptr, size_t size)
{
	//Handle size=0 case trivially
	if(size == 0)
	{
		return;
	}
	
	//Lock kernel-space while freeing
	hal_spl_lock(&kspace_spl);
	
	//Pointer should have come from kspace_alloc, so should be page-aligned.
	size_t pagesize = hal_frame_size();
	uintptr_t region_start = (uintptr_t)ptr;
	KASSERT( (region_start % pagesize) == 0 );
	
	//Find how many pages we'll be freeing.
	//We expand the requested size to page-length in kspace_alloc, so we do the same here.
	size_t pages_to_free = (size + (pagesize - 1)) / pagesize;
	size_t size_to_free = pages_to_free * pagesize;
	for(uintptr_t free_virt = region_start; free_virt < region_start + size_to_free; free_virt += pagesize)
	{
		hal_frame_id_t old_frame = hal_kspc_get(free_virt);
		KASSERT(old_frame != HAL_FRAME_ID_INVALID);
		
		hal_kspc_set(free_virt, HAL_FRAME_ID_INVALID);
		hal_frame_free(old_frame);
	}
	
	//Success
	hal_spl_unlock(&kspace_spl);
}

void *kspace_phys_map(hal_frame_id_t paddr, size_t size)
{
	//Handle size=0 case trivially
	if(size == 0)
	{
		return NULL;
	}
		
	//Lock kernel-space while allocating
	hal_spl_lock(&kspace_spl);
	
	//Figure out how many pages we'll need to allocate to cover this region
	size_t pagesize = hal_frame_size();
	size_t pages_needed = (size + (pagesize - 1)) / pagesize;
	
	//Find a free area of kernel-space containing that number of pages, plus a guard page on each end.
	size_t contiguous_start = kspace_findfree(pages_needed * pagesize, pagesize);
	if(contiguous_start == 0)
	{
		//Failed to find enough contiguous free virtual space.
		hal_spl_unlock(&kspace_spl);
		return NULL;
	}
	
	//Okay, we know where we'll put the new allocation in kernel space.
	//Map with the given physical region.
	uintptr_t alloc_start = contiguous_start;
	uintptr_t alloc_size = pages_needed * pagesize;
	for(uintptr_t frame_iter = alloc_start; frame_iter < alloc_start + alloc_size; frame_iter += pagesize)
	{
		hal_kspc_set(frame_iter, paddr + (frame_iter - alloc_start));
	}
	
	//Success! Return the beginning of the allocation.
	hal_spl_unlock(&kspace_spl);
	return (void*)alloc_start;	
}

void kspace_phys_unmap(void *vaddr, size_t size)
{
	//Handle size=0 case trivially
	if(size == 0)
	{
		return;
	}
	
	//Lock kernel-space while freeing
	hal_spl_lock(&kspace_spl);
	
	//Pointer should have come from kspace_phys_map, so should be page-aligned.
	size_t pagesize = hal_frame_size();
	uintptr_t region_start = (uintptr_t)vaddr;
	KASSERT( (region_start % pagesize) == 0 );
	
	//Find how many pages we'll be freeing.
	//We expand the requested size to page-length in kspace_phys_map, so we do the same here.
	size_t pages_to_free = (size + (pagesize - 1)) / pagesize;
	size_t size_to_free = pages_to_free * pagesize;
	for(uintptr_t free_virt = region_start; free_virt < region_start + size_to_free; free_virt += pagesize)
	{
		KASSERT(hal_kspc_get(free_virt) != HAL_FRAME_ID_INVALID);
		hal_kspc_set(free_virt, HAL_FRAME_ID_INVALID);
	}
	
	//Success
	hal_spl_unlock(&kspace_spl);	
}

