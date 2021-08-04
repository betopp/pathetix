//mem.c
//Memory space tracking in kernel
//Bryan E. Topp <betopp@betopp.com> 2021

#include "mem.h"
#include "kassert.h"
#include "kspace.h"

#include "hal_frame.h"

#include <errno.h>
#include <stddef.h>

mem_space_t *mem_space_new(void)
{
	mem_space_t *retval = kspace_alloc(sizeof(mem_space_t), alignof(mem_space_t));
	if(retval == NULL)
		return NULL;
	
	retval->uspc = hal_uspc_new();
	if(retval->uspc == HAL_USPC_ID_INVALID)
	{
		kspace_free(retval, sizeof(mem_space_t));
		return NULL;
	}
	
	return retval;
}

mem_space_t *mem_space_fork(mem_space_t *old)
{
	const size_t pagesize = hal_frame_size();
	
	mem_space_t *forked = mem_space_new();
	if(forked == NULL)
		return NULL;
	
	for(int mm = 0; mm < MEM_SEG_MAX; mm++)
	{
		mem_seg_t *oldseg = &(old->seg_array[mm]);
		if(oldseg->end <= 0)
			continue;
		
		//Todo - distinguish shared memory?
		int add_err = mem_space_add(forked, oldseg->start, oldseg->end - oldseg->start, oldseg->prot);
		if(add_err < 0)
		{
			mem_space_delete(forked);
			return NULL;
		}
		
		for(uintptr_t aa = oldseg->start; aa < oldseg->end; aa += pagesize)
		{
			hal_frame_id_t frame_old = hal_uspc_get(old->uspc, aa);
			hal_frame_id_t frame_new = hal_uspc_get(forked->uspc, aa);
			KASSERT(frame_old != HAL_FRAME_ID_INVALID);
			KASSERT(frame_old % pagesize == 0);
			KASSERT(frame_new != HAL_FRAME_ID_INVALID);
			KASSERT(frame_new % pagesize == 0);
			hal_frame_copy(frame_new, frame_old);
		}
	}
	
	return forked;
}

void mem_space_delete(mem_space_t *mptr)
{
	size_t pagesize = hal_frame_size();
	
	for(int mm = 0; mm < MEM_SEG_MAX; mm++)
	{
		if(mptr->seg_array[mm].end > 0)
		{
			uintptr_t start = mptr->seg_array[mm].start;
			uintptr_t end = mptr->seg_array[mm].end;
			KASSERT(start % pagesize == 0);
			KASSERT(end % pagesize == 0);
			for(uintptr_t aa = start; aa < end; aa += pagesize)
			{
				//Todo - check that these frames are private before freeing them
				hal_frame_id_t oldframe = hal_uspc_get(mptr->uspc, aa);
				KASSERT(oldframe != HAL_FRAME_ID_INVALID);
				hal_uspc_set(mptr->uspc, aa, HAL_FRAME_ID_INVALID);
				hal_frame_free(oldframe);
			}
		}
		
		mptr->seg_array[mm].start = 0;
		mptr->seg_array[mm].end = 0;
	}
	
	hal_uspc_delete(mptr->uspc);
	kspace_free(mptr, sizeof(mem_space_t));
}

int mem_space_add(mem_space_t *mptr, uintptr_t addr, size_t size, int prot)
{
	//Address and length must be page-aligned
	size_t pagesize = hal_frame_size();
	if( (addr % pagesize) != 0 )
		return -EINVAL;
	if( (size % pagesize) != 0 )
		return -EINVAL;
	
	//Make sure there's room. If the last array entry is used, there's no room.
	if(mptr->seg_array[MEM_SEG_MAX - 1].end > 0)
		return -ENOMEM;
		
	//Make sure the segment doesn't overlap any existing ones
	int insertidx = -1;
	uintptr_t end = addr + size;
	for(int mm = 0; mm < MEM_SEG_MAX; mm++)
	{
		uintptr_t existing_addr = mptr->seg_array[mm].start;
		uintptr_t existing_end = mptr->seg_array[mm].end;
		if(addr < existing_end && end > existing_addr)
		{
			//Attempted new mapping would overlap an existing one.
			return -ENOMEM;
		}
		
		//Existing mappings with addresses lower than the new one can stay in place.
		//The first mapping with an address greater than the new one will get bumped.
		if(insertidx == -1)
		{
			if( (existing_addr > addr) || (existing_end == 0) )
			{
				insertidx = mm;
				break;
			}
		}
	}
	
	KASSERT(insertidx >= 0);
	KASSERT(insertidx < MEM_SEG_MAX);
	
	//Try to allocate and map frames to back the region.
	for(uintptr_t aa = addr; aa < end; aa += pagesize)
	{
		hal_frame_id_t frame = hal_frame_alloc();
		if(frame != HAL_FRAME_ID_INVALID)
		{
			KASSERT( (aa % pagesize) == 0 );
			KASSERT( (frame % pagesize) == 0 );
			int ins_err = hal_uspc_set(mptr->uspc, aa, frame); //Todo - set protection
			if(ins_err == 0)
			{
				//Success, keep adding frames
				continue;
			}
		}
		
		//Failed to allocate and/or map the frame.
		
		//If we had allocated a frame but couldn't map it, free it.
		if(frame != HAL_FRAME_ID_INVALID)
		{
			hal_frame_free(frame);
			frame = HAL_FRAME_ID_INVALID;
		}
		
		//Unwind any that we did actually map.
		while(aa > addr)
		{
			aa -= pagesize;
			hal_frame_id_t oldframe = hal_uspc_get(mptr->uspc, aa);
			KASSERT(oldframe != HAL_FRAME_ID_INVALID);
			hal_uspc_set(mptr->uspc, aa, HAL_FRAME_ID_INVALID);
			hal_frame_free(oldframe);
		}
		
		//Return that we ran out of memory (note - at this point, we didn't add a mem_seg_t yet.)
		return -ENOMEM;
	}
	
	//Scoot existing array entries down to make room, and store the new bookkeeping.
	KASSERT(mptr->seg_array[MEM_SEG_MAX-1].end == 0);
	for(int ss = MEM_SEG_MAX - 1; ss > insertidx; ss--)
	{
		mptr->seg_array[ss] = mptr->seg_array[ss-1];
	}
	
	//Store bookkeeping
	KASSERT(insertidx >= 0 && insertidx < MEM_SEG_MAX);
	mptr->seg_array[insertidx].start = addr;
	mptr->seg_array[insertidx].end = end;
	mptr->seg_array[insertidx].prot = prot;
	
	//Run through bookkeeping and see if there's any regions that can be combined
	for(int ss = 0; ss < MEM_SEG_MAX - 1; ss++)
	{
		mem_seg_t *cur = &(mptr->seg_array[ss]);
		mem_seg_t *next = &(mptr->seg_array[ss+1]);
		
		KASSERT(cur->end != 0); //Should have at least one segment, as we just added one
		KASSERT(cur->start < cur->end);
		
		if(next->end == 0)
			break; //No following segment to combine with
		
		KASSERT(cur->end <= next->start);
		KASSERT(next->start < next->end);
		
		if(cur->end == next->start)
		{
			if(cur->prot == next->prot)
			{
				//(Todo - check if they're the same file, once we support that)
				//Can combine the two segments.
				cur->end = next->end;
				for(int ii = ss + 1; ii < MEM_SEG_MAX - 1; ii++) //Scoot up, clobbering "next"
				{
					mptr->seg_array[ii] = mptr->seg_array[ii+1];
				}
				mptr->seg_array[MEM_SEG_MAX-1].start = 0; //Last entry now is free
				mptr->seg_array[MEM_SEG_MAX-1].end = 0;
				mptr->seg_array[MEM_SEG_MAX-1].prot = 0;
				
				//Look again at this same segment next loop
				ss--;
				continue;
			}
		}
	}
	
	//Success
	return insertidx;
}

int mem_space_clear(mem_space_t *mptr, uintptr_t addr, size_t size)
{
	size_t pagesize = hal_frame_size();
	if( (addr % pagesize) || (size % pagesize) )
		return -EINVAL; //Non page aligned
	
	//Update bookkeeping for removing this range - change all affected segments
	uintptr_t remove_start = addr;
	uintptr_t remove_end = addr + size;
	for(int ss = 0; ss < MEM_SEG_MAX; ss++)
	{
		if(mptr->seg_array[ss].end <= 0)
			break; //No further segments
		
		uintptr_t seg_start = mptr->seg_array[ss].start;
		uintptr_t seg_end = mptr->seg_array[ss].end;
		
		if(seg_start >= remove_end)
		{
			//No overlap - this segment unchanged
			continue;
		}
		
		if(seg_end <= remove_start)
		{
			//No overlap - this segment unchanged
			continue;
		}
		
		if(seg_start < remove_start && seg_end > remove_end)
		{
			//"Chopping" case. The removed region is totally inside this segment.
			//Further, parts of the segment will remain at both ends once the operation is finished.
			//See if we have room for the additional bookkeeping (two segments instead of one).
			if(mptr->seg_array[MEM_SEG_MAX-1].end > 0)
			{
				//No room for more bookkeeping. Can't chop the segment in two.
				return -ENOMEM;
			}			
			
			//Scoot all further segments down
			for(int ii = MEM_SEG_MAX - 1; ii > ss; ii--)
			{
				mptr->seg_array[ii] = mptr->seg_array[ii-1];
			}
			
			//Store the bookkeeping for the two pieces
			mptr->seg_array[ss].start = seg_start;
			mptr->seg_array[ss].end = remove_start;
			mptr->seg_array[ss+1].start = remove_end;
			mptr->seg_array[ss+1].end = seg_end;
			
			//Segments are nonoverlapping, so this is the only segment modified.
			break;
		}
		
		if(seg_start >= remove_start && seg_end <= remove_end)
		{
			//Segment is totally removed. Remove it and scoot all bookkeeping up.
			for(int ii = ss; ii < MEM_SEG_MAX - 1; ii++)
			{
				mptr->seg_array[ii] = mptr->seg_array[ii+1];
			}
			mptr->seg_array[MEM_SEG_MAX-1].start = 0;
			mptr->seg_array[MEM_SEG_MAX-1].end = 0;
			
			//More segments may be affected
			continue;
		}
		
		if(remove_start <= seg_start)
		{
			//Removing the beginning of the segment, leaving the end
			KASSERT(remove_end > seg_start);
			KASSERT(remove_end < seg_end);
			mptr->seg_array[ss].start = remove_end;
			mptr->seg_array[ss].end = seg_end;
			continue;
		}
		
		if(remove_end >= seg_end)
		{
			//Removing the end of the segment, leaving the beginning
			KASSERT(remove_start > seg_start);
			KASSERT(remove_start < seg_end);
			mptr->seg_array[ss].start = seg_start;
			mptr->seg_array[ss].end = remove_start;
			continue;
		}
		
		//All cases should have been handled above
		KASSERT(0);		
	}

	//Unmap the pages
	for(uintptr_t aa = addr; aa < addr + size; aa += pagesize)
	{
		hal_frame_id_t fr = hal_uspc_get(mptr->uspc, aa);
		if(fr != HAL_FRAME_ID_INVALID)
		{
			hal_uspc_set(mptr->uspc, aa, HAL_FRAME_ID_INVALID);
			//Todo - don't free the frame if it came from a file
			hal_frame_free(fr);
		}
	}	
	
	return 0;
}

intptr_t mem_space_avail(mem_space_t *mptr, uintptr_t around, size_t size)
{
	if(size <= 0)
		return EINVAL;
	
	//Get overall bounds of userspace from HAL
	uintptr_t uspc_start = 0;
	uintptr_t uspc_end = 0;
	hal_uspc_bound(&uspc_start, &uspc_end);
	
	//Segments are stored in-order.
	//This implies that the free spaces can be found in the gaps between them as stored.
	//Search the free regions for a region of at least the given size, closest to the given address.
	
	//If there's nothing at all mapped, then we have a trivial problem
	if(mptr->seg_array[0].end == 0)
	{
		if( (around >= uspc_start) && ((around + size) <= uspc_end) )
			return around;
		else if(size <= uspc_end - uspc_start)
			return uspc_start;
		else
			return 0;
	}
	
	//Okay, we have at least one segment mapped already.
	//There's more than one place we might put this. Search for the area closest to the requested address.
	uintptr_t best_start = 0;
	uintptr_t best_diff = ~0ull;
	
	//Consider the gaps around each segment
	for(int gg = 0; gg < MEM_SEG_MAX + 1; gg++)
	{
		//Gap gg is the space between memory segments gg-1 and gg
		//If gg is 0 then it's the space from the beginning to segment gg (the first segment)
		//If gg is MEM_SEG_MAX then it's the space from segment gg-1 (the last segment) to the end
		uintptr_t gap_start = (gg == 0) ? uspc_start : (mptr->seg_array[gg-1].end);
		uintptr_t gap_end = ( (gg == MEM_SEG_MAX) || (mptr->seg_array[gg].end == 0) ) ? uspc_end : (mptr->seg_array[gg].start);
		
		//Check if the gap is big enough for the proposed region at all
		if(gap_end - gap_start < size)
			continue; //Not enough room
		
		//See what placement would be closest to the proposed address
		uintptr_t best_start_in_gap = 0;
		if(around < gap_start)
		{
			//Wanted an address before the gap - closest we'll get is the beginning
			best_start_in_gap = gap_start;
		}
		else if(around + size > gap_end)
		{
			//Wanted a range that ends after the gap - closest we'll get is the end
			best_start_in_gap = gap_end - size;
		}
		else
		{
			//Can satisfy exactly the request in this gap
			best_start_in_gap = around;
		}
		
		uintptr_t diff = (best_start_in_gap > around) ? (best_start_in_gap - around) : (around - best_start_in_gap);
		if(diff < best_diff)
		{
			best_start = best_start_in_gap;
			best_diff = diff;
		}
		
		//If there's no more memory segments, there's no more gaps
		if(mptr->seg_array[gg].end == 0)
			break;
	}
	
	if(best_start == 0)
		return -ENOMEM;
	
	return (intptr_t)best_start;
}


