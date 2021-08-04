//malloc.c
//malloc and friends for standard library
//Bryan E. Topp <betopp@betopp.com> 2021

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <px.h>
#include "rb.h"
#include "spl.h"

//Spinlock protecting malloc bookkeeping.
static _spl_t _malloc_spl;

//Whether malloc bookkeeping has been initialized.
static int _malloc_ready;

//Tree containing allocatable ranges sorted by address.
static _rb_tree_t _malloc_addr_tree;

//Tree containing allocatable ranges sorted by size.
static _rb_tree_t _malloc_size_tree = { .allowdup = true };

//Bookkeeping stored at the beginning of each allocatable range.
typedef struct _malloc_free_item_s
{
	//Entry in address-keyed tree
	_rb_item_t addr_item;
	
	//Entry in size-keyed tree
	_rb_item_t size_item;
	
} _malloc_free_item_t;

//Bookkeeping stored at the beginning of each allocated range.
typedef struct _malloc_used_item_s
{
	//Size of range allocated, including this structure.
	size_t size;
	
	//Magic number.
	uint64_t magic;
	
} _malloc_used_item_t;

//Magic number stored in used-item bookkeeping.
#define MALLOC_USED_MAGIC 0x737564656d5f6d65 //used_mem

//Returns the user-visible pointer for the given used-item bookkeeping.
static void *_malloc_userptr_for_item(_malloc_used_item_t *item)
{
	return (void*)((uintptr_t)(item) + sizeof(_malloc_used_item_t));
}

//Returns the used-item bookkeeping for the given user-visible pointer.
static _malloc_used_item_t *_malloc_item_for_userptr(void *userptr)
{
	return (_malloc_used_item_t*)((uintptr_t)(userptr) - sizeof(_malloc_used_item_t));
}

//Sets up initial malloc bookkeeping.
static void _malloc_init(void)
{	
	_malloc_ready = 1;
}

//Locks malloc bookkeeping, and initializes if necessary
static void _malloc_lock(void)
{
	_spl_lock(&_malloc_spl);
	
	if(!_malloc_ready)
		_malloc_init();
}

//Unlocks malloc bookkeeping.
static void _malloc_unlock(void)
{
	_spl_unlock(&_malloc_spl);
}

void *malloc(size_t size)
{	
	_malloc_lock();
	
	//For the given user allocation, how big a region do we actually need?
	size_t size_needed = size + sizeof(_malloc_used_item_t);
	if(size_needed < sizeof(_malloc_free_item_t))
	{
		//When we later free this allocation, it might be alone between two allocated regions.
		//In that case, it will need room for free-item bookkeeping.
		//Don't ever allocate a region smaller than the free-item bookkeeping.
		size_needed = sizeof(_malloc_free_item_t);
	}
	
	//Find a region of approximately that size in the tree of free regions by size.
	_rb_item_t *rbitem = _rb_findabout(&_malloc_size_tree, size_needed);
	
	//Walk back in the tree until we find the smallest entry that will work.
	_malloc_free_item_t *item_found = NULL;
	while(rbitem != NULL)
	{
		//These entries are in a tree keyed with the size of the region.
		//So look at the key to determine if the region is big enough.
		if(rbitem->id == size_needed)
		{
			//Exactly the right size.
			item_found = (_malloc_free_item_t*)(rbitem->userptr);
			break;
		}
		else if(rbitem->id < size_needed)
		{
			//We're looking at a region that's too small.
			//Try to find a bigger region.
			rbitem = _rb_next(&_malloc_size_tree, rbitem);
			if(rbitem != NULL && rbitem->id >= size_needed)
			{
				//We found a region that was too small, but then the next bigger region is big enough.
				//It must be the smallest that would suffice.
				item_found = (_malloc_free_item_t*)(rbitem->userptr);
				break;
			}
		}
		else
		{
			//We're looking at a region that's bigger than we need.
			//See if there's any smaller region we can check.
			if(_rb_prev(&_malloc_size_tree, rbitem) == NULL)
			{
				//This region is bigger than we need but there's no smaller regions.
				item_found = (_malloc_free_item_t*)(rbitem->userptr);
				break;
			}
			
			//Continue looking at the smaller region.
			rbitem = _rb_prev(&_malloc_size_tree, rbitem);
		}
	}
	
	//If we didn't find any suitably-sized region, we can't satisfy the allocation from our current regions.
	//Try to allocate a new region.
	if(item_found == NULL)
	{
		//Min allocation size so we don't pester the kernel too much
		size_t req = 65536;
		if(size_needed > req)
			req = size_needed;
		
		//Find room in our memory space for new pages
		intptr_t avail = px_mem_avail(0, req);
		if(avail < 0)
		{
			//No room in memory map for a new allocation...?
			_malloc_unlock();
			return NULL;
		}
		
		//Try to map new pages there
		int mapped = px_mem_anon(avail, req, PX_MEM_R | PX_MEM_W);
		if(mapped < 0)
		{
			//Failed to map more memory
			_malloc_unlock();
			return NULL;
		}
		
		//Make bookkeeping for the new allocation, and use it.
		_malloc_free_item_t *new_free_item = (_malloc_free_item_t*)(avail);
		_rb_insert(&_malloc_addr_tree, &(new_free_item->addr_item), avail, new_free_item);
		_rb_insert(&_malloc_size_tree, &(new_free_item->size_item), req, new_free_item);
		item_found = new_free_item;
	}
	
	//Alright, we know what free region will be used to satisfy this allocation.
	//Set aside its location and destroy the old free-region bookkeeping.
	uintptr_t region_addr = item_found->addr_item.id;
	size_t region_size = item_found->size_item.id;
	_rb_remove(&_malloc_addr_tree, &(item_found->addr_item));
	_rb_remove(&_malloc_size_tree, &(item_found->size_item));
	item_found = NULL;
	
	//Figure out how much will remain free, after we consume our space.
	size_t size_remain = region_size - size_needed;
	uintptr_t addr_remain = region_addr + size_needed;
	if(size_remain >= sizeof(_malloc_free_item_t))
	{
		//We can satisfy this allocation and still have enough room to book-keep the remaining free space.
		
		//Make bookkeeping for the allocated region, of just the size we need.
		_malloc_used_item_t *used_item = (_malloc_used_item_t *)(region_addr);
		used_item->size = size_needed;
		used_item->magic = MALLOC_USED_MAGIC;
		
		//Make a new entry for the remaining region and put it in the tree.
		_malloc_free_item_t *remaining_free_item = (_malloc_free_item_t *)(addr_remain);
		memset(remaining_free_item, 0, sizeof(*remaining_free_item));
		_rb_insert(&_malloc_addr_tree, &(remaining_free_item->addr_item), addr_remain, remaining_free_item);
		_rb_insert(&_malloc_size_tree, &(remaining_free_item->size_item), size_remain, remaining_free_item);
		
		//Return a pointer just after the used-region bookkeeping.
		_malloc_unlock();
		return _malloc_userptr_for_item(used_item);
	}
	else
	{
		//There's not enough room to store bookkeeping for the remaining free bytes.
		//Consume the entire region instead in this allocation.
		_malloc_used_item_t *used_item = (_malloc_used_item_t *)(region_addr);
		used_item->size = region_size;
		used_item->magic = MALLOC_USED_MAGIC;
		
		//Return a pointer just after the used-region bookkeeping.
		_malloc_unlock();
		return _malloc_userptr_for_item(used_item);
	}
}

void free(void *addr)
{
	_malloc_lock();
	
	//Find the bookkeeping that precedes the user's pointer, to learn the size of the allocation.
	_malloc_used_item_t *item_ptr = _malloc_item_for_userptr(addr);
	assert(item_ptr->size >= sizeof(_malloc_used_item_t));
	assert(item_ptr->magic == MALLOC_USED_MAGIC);
	size_t freeing_size = item_ptr->size;
	uintptr_t freeing_addr = (uintptr_t)(item_ptr);
	
	//For security, poison the whole region being freed.
	memset((void*)freeing_addr, 0xBA, freeing_size);
	
	//Make a new free-region bookkeeping structure for the region freed.
	//(We never allocate less space than it takes to hold this bookkeeping.)
	assert(freeing_size >= sizeof(_malloc_free_item_t));
	_malloc_free_item_t *free_item = (_malloc_free_item_t*)(freeing_addr);
	
	memset(free_item, 0, sizeof(*free_item));
	_rb_insert(&_malloc_addr_tree, &(free_item->addr_item), freeing_addr, free_item);
	_rb_insert(&_malloc_size_tree, &(free_item->size_item), freeing_size, free_item);
	
	//We may have freed a region that is adjacent to another free region.
	//Coalesce free regions that are adjacent in memory, representing a larger region that is all free.
	while(1)
	{
		//See if the freed range has a previous range that touches it.
		_rb_item_t *prev_rbitem = _rb_prev(&_malloc_addr_tree, &(free_item->addr_item));
		if(prev_rbitem != NULL)
		{
			_malloc_free_item_t *prev_item = (_malloc_free_item_t*)(prev_rbitem->userptr);
			uintptr_t prev_ends = prev_item->addr_item.id + prev_item->size_item.id;
			assert(prev_ends <= free_item->addr_item.id); //Two ranges should never overlap
			if(prev_ends == free_item->addr_item.id)
			{
				//The previous free region ends exactly where this one begins.
				//Coalesce them, putting the bookkeeping at the beginning of the combined region.
				_malloc_free_item_t *combined_item = prev_item;
				uintptr_t combined_addr = prev_item->addr_item.id;
				size_t combined_size = prev_item->size_item.id + free_item->size_item.id;
				
				_rb_remove(&_malloc_addr_tree, &(prev_item->addr_item));
				_rb_remove(&_malloc_size_tree, &(prev_item->size_item));
				
				_rb_remove(&_malloc_addr_tree, &(free_item->addr_item));
				_rb_remove(&_malloc_size_tree, &(free_item->size_item));
				
				_rb_insert(&_malloc_addr_tree, &(combined_item->addr_item), combined_addr, combined_item);
				_rb_insert(&_malloc_size_tree, &(combined_item->size_item), combined_size, combined_item);
				
				//Keep looking for more regions to coalesce around the combined region.
				free_item = combined_item;
				continue;
			}
		}
		
		//See if the freed range has a following range that it touches.
		_rb_item_t *next_rbitem = _rb_next(&_malloc_addr_tree, &(free_item->addr_item));
		if(next_rbitem != NULL)
		{
			_malloc_free_item_t *next_item = (_malloc_free_item_t*)(next_rbitem->userptr);
			uintptr_t free_ends = free_item->addr_item.id + free_item->size_item.id;
			assert(free_ends <= next_item->addr_item.id); //Two ranges should never overlap
			if(free_ends == next_item->addr_item.id)
			{
				//This free region ends exactly where the next one begins.
				//Coalesce them, putting the bookkeeping at the beginning of the combined region.
				_malloc_free_item_t *combined_item = free_item;
				uintptr_t combined_addr = free_item->addr_item.id;
				size_t combined_size = free_item->size_item.id + next_item->size_item.id;
				
				_rb_remove(&_malloc_addr_tree, &(free_item->addr_item));
				_rb_remove(&_malloc_size_tree, &(free_item->size_item));
				
				_rb_remove(&_malloc_addr_tree, &(next_item->addr_item));
				_rb_remove(&_malloc_size_tree, &(next_item->size_item));
				
				_rb_insert(&_malloc_addr_tree, &(combined_item->addr_item), combined_addr, combined_item);
				_rb_insert(&_malloc_size_tree, &(combined_item->size_item), combined_size, combined_item);
				
				//Keep looking for more regions to coalesce around the combined region.
				free_item = combined_item;
				continue;
			}
		}		
		
		//If we weren't able to coalesce any regions, stop looking.
		break;
	}
	
	_malloc_unlock();
}


void *calloc(size_t num, size_t size)
{
	void *area = malloc(num*size);
	if(area == NULL)
		return NULL;
	
	memset(area, 0, num*size);
	return area;
}

void *realloc(void *ptr, size_t size)
{
	if(ptr == NULL)
		return malloc(size);
	
	//Find size of existing allocation, for limiting how much we copy
	_malloc_lock();
	_malloc_used_item_t *used_item = _malloc_item_for_userptr(ptr);
	assert(used_item->size >= sizeof(_malloc_used_item_t));
	assert(used_item->magic == MALLOC_USED_MAGIC);
	size_t old_size = used_item->size - sizeof(_malloc_used_item_t); //Size, as stored, includes bookkeeping
	_malloc_unlock();
	
	//Make new allocation
	void *newptr = malloc(size);
	if(newptr == NULL)
		return NULL;
	
	//Copy the data - using the smaller of the two sizes
	size_t to_copy = (old_size < size) ? old_size : size;
	memcpy(newptr, ptr, to_copy);
	
	//Free the old allocation and return the new one.
	free(ptr);
	return newptr;
}
