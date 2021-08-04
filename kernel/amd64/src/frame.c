//frame.c
//Physical memory allocator
//Bryan E. Topp <betopp@betopp.com> 2021

#include "pmem.h"
#include "hal_frame.h"
#include "hal_spl.h"
#include <stdint.h>
#include <sys/types.h>

//Free-list of frames
static hal_frame_id_t frame_head;

//Number of free frames
static size_t frame_count;

//Spinlock protecting frame allocator
static hal_spl_t frame_spl;

size_t hal_frame_size(void)
{
	//Always small-pages
	return 4096;
}

hal_frame_id_t hal_frame_alloc(void)
{
	hal_spl_lock(&frame_spl);
	
	//Check if there's any frames free to allocate
	if(frame_head == 0)
	{
		hal_spl_unlock(&frame_spl);
		return 0;
	}
	
	//Return the frame that was previously on the head of the free-list.
	//Advance the head to the next entry in the list.
	hal_frame_id_t retval = frame_head;
	frame_head = pmem_read(frame_head);
	
	frame_count--;
	hal_spl_unlock(&frame_spl);
	
	pmem_clrframe(retval); //Zero everything before allowing it to be used. Paranoid? Maybe.
	return retval;
}

void hal_frame_free(hal_frame_id_t frame)
{
	hal_spl_lock(&frame_spl);
	
	//Write the old head of the free-list into the frame we're freeing.
	//The frame we're freeing becomes the head of the list.
	pmem_write(frame, frame_head);
	frame_head = frame;
	frame_count++;
	
	hal_spl_unlock(&frame_spl);
}

size_t hal_frame_count(void)
{
	hal_spl_lock(&frame_spl);
	size_t val = frame_count;
	hal_spl_unlock(&frame_spl);
	return val;
}


//RAM info set aside from Multiboot
typedef struct multiboot_mmap_info_s
{
	uint32_t next;
	uint64_t base;
	uint64_t length;
	uint32_t type;
} __attribute__((packed)) multiboot_mmap_info_t;
extern const multiboot_mmap_info_t multiboot_mmap_storage[];
extern const size_t multiboot_mmap_size;

//Module info set aside from Multiboot
typedef struct multiboot_modinfo_s
{
	uint32_t start;
	uint32_t end;
	uint32_t stringptr;
	uint32_t unused;
} multiboot_modinfo_t;
extern const multiboot_modinfo_t multiboot_modinfo_storage[];
extern const size_t multiboot_modinfo_size;

//Symbols from linker-script about placement of kernel
extern char _MULTIBOOT_ZERO_END;

//Runs through memory regions set-aside from Multiboot loader.
//Marks RAM free as appropriate.
void frame_free_multiboot()
{
	//Iterate through memory map info set aside from Multiboot.
	//The entries have a next-offset as their first member, so may not be uniformly-sized and packed.
	const char *mmap_byte_ptr = (const char*)(multiboot_mmap_storage);
	size_t mmap_offset = 0;
	while(mmap_offset < multiboot_mmap_size)
	{
		//Get next info entry
		const multiboot_mmap_info_t *info = (const multiboot_mmap_info_t*)(mmap_byte_ptr + mmap_offset);
		if(info->type == 1)
		{
			//This is usable RAM. Try to add its frames.
			uintptr_t range_start = info->base;
			uintptr_t range_end = info->base + info->length;
			
			//Cap the start of the range. 
			
			//Don't allow ranges that start before the end-of-kernel.
			if(range_start < (uintptr_t)(&_MULTIBOOT_ZERO_END))
				range_start = (uintptr_t)(&_MULTIBOOT_ZERO_END);
			
			//Don't allow ranges that start before the end of the last module.
			for(uint32_t mm = 0; mm < multiboot_modinfo_size / 16; mm++)
			{
				if(range_start < multiboot_modinfo_storage[mm].end)
					range_start = multiboot_modinfo_storage[mm].end;
			}
			
			//Round the base up to a page boundary
			range_start += 0xFFF;
			range_start &= 0xFFFFFFFFFFFFF000;
			
			//Round the end down to a page boundary
			range_end &= 0xFFFFFFFFFFFFF000;
			
			//Free all pages within the range
			for(uintptr_t pp = range_start; pp < range_end; pp += 4096)
			{
				hal_frame_free(pp);
			}
		}
		
		//Advance to next map entry
		mmap_offset += info->next + 4;
	}
}

