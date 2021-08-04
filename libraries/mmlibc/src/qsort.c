//qsort.c
//Quicksort implementation for libc
//Bryan E. Topp <betopp@betopp.com> 2021

#include <stdlib.h>
#include <stddef.h>
#include <assert.h>

//Macro for computing addresses of a quicksort array element.
#define QSORT_MEMB(base, size, which) ((void*)(((unsigned char*)(base)) + ((size) * (which))))

//Swaps array elements in a quicksort.
static void _qsort_swap(void *base, size_t size, size_t first, size_t second)
{
	//Swap byte-at-a-time so we don't need to alloc space for a temporary of arbitrary size.
	unsigned char *first_bytes = ((unsigned char *)base) + (size * first);
	unsigned char *second_bytes = ((unsigned char *)base) + (size * second);
	for(size_t bb = 0; bb < size; bb++)
	{
		unsigned char byte_from_first = first_bytes[bb];
		unsigned char byte_from_second = second_bytes[bb];
		
		first_bytes[bb] = byte_from_second;
		second_bytes[bb] = byte_from_first;
	}
}

void qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *))
{
	if(nmemb < 2)
	{
		//Less than 2 elements. No sorting to be done.
		return;
	}
	
	//Pick a halfway element as a pivot.
	//Classic qsort picks the first or last... but then has pathological behavior in the already-sorted case.
	//This way we don't go O(n^2) if the array is already sorted.
	//We can still hit the same type of failure, but in a less-common case.
	size_t elem_pivot = nmemb / 2;
	
	//Swap elements less-than the pivot to the beginning.
	//Swap elements greater-than the pivot to the end.
	size_t elem_front = 0;
	size_t elem_back = nmemb - 1;
	while(1)
	{
		//Advance past all beginning-of-array elements that are already less than the pivot.
		while(elem_front < elem_back)
		{
			int front_compar = (*compar)(QSORT_MEMB(base, size, elem_front), QSORT_MEMB(base, size, elem_pivot));
			if(front_compar < 0)
			{
				//This "front" element is already in the right place.
				elem_front++;
			}
			else
			{
				//This "front" element needs to be swapped.
				break;
			}
		}
		
		//Advance (backwards) past all end-of-array elements that are already greater than the pivot.
		while(elem_front < elem_back)
		{
			int back_compar = (*compar)(QSORT_MEMB(base, size, elem_back), QSORT_MEMB(base, size, elem_pivot));
			if(back_compar > 0)
			{
				//This "back" element is already in the right place.
				elem_back--;
			}
			else
			{
				//This "back" element needs to be swapped.
				break;
			}
		}
		
		//So we've advanced past all the correct elements at the beginning and end of array.
		//We might be totally done, if the two element-pointers have overlapped.
		if(elem_front >= elem_back)
		{
			//All elements have been pivoted.
			break;
		}
		
		//If we're not totally done, then the "front" element and the "back" element are both in the wrong place.
		_qsort_swap(base, size, elem_front, elem_back);
		
		//Keep track of where the pivot went, if we swapped the pivot element.
		if(elem_pivot == elem_front)
		{
			elem_pivot = elem_back;
		}
		else if(elem_pivot == elem_back)
		{
			elem_pivot = elem_front;
		}
		
		//Technically, here, we could advance elem_front and elem_back, as we just put them in the right place.
		//But the code looks cleaner if we just compare them again on the next iteration.
	}
	
	//We've now sorted the array into a greater-than-the-pivot and a less-than-the-pivot sub-array.
	//Now recurse and sort those two sub-arrays.
	//Note that we never need to sort the pivot again. This guarantees each further recursion will sort fewer elements.
	//elem_front and elem_back should both be pointing at the pivot element.
	assert(elem_front == elem_pivot);
	assert(elem_back == elem_pivot);
	
	if(elem_pivot <= 0)
	{
		//Whole array was greater-than-pivot.
		qsort(QSORT_MEMB(base, size, 1), nmemb - 1, size, compar);
	}
	else if(elem_pivot >= (nmemb - 1))
	{
		//Whole array was less-than-pivot.
		qsort(QSORT_MEMB(base, size, 0), nmemb - 1, size, compar);
	}
	else
	{
		//Need to sort both sides
		qsort(QSORT_MEMB(base, size, 0),         elem_front,        size, compar);
		qsort(QSORT_MEMB(base, size, elem_back), nmemb - elem_back, size, compar);
	}
}
