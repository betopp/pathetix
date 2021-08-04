//hal_frame.h
//Allocating and freeing physical frames
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef HAL_FRAME_H
#define HAL_FRAME_H

#include <stdint.h>
#include <sys/types.h>

//Identifies a physical frame.
typedef uint64_t hal_frame_id_t;
#define HAL_FRAME_ID_INVALID ((hal_frame_id_t)0)

//Returns the size of physical frames.
size_t hal_frame_size(void);

//Frees the given frame back to the frame allocator.
void hal_frame_free(hal_frame_id_t paddr);

//Allocates a frame from the frame allocator.
hal_frame_id_t hal_frame_alloc(void);

//Returns how many free frames are currently available.
size_t hal_frame_count(void);

//Copies a physical frame of memory
void hal_frame_copy(hal_frame_id_t dst, hal_frame_id_t src);

#endif //HAL_FRAME_H
