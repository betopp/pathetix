//mmlibc/include/mmbits/struct_typed_mem_info.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _STRUCT_TYPED_MEM_INFO_H
#define _STRUCT_TYPED_MEM_INFO_H

#include <mmbits/typedef_size.h>

struct posix_typed_mem_info
{
	size_t posix_tmi_length; //Maximum length allocatable from a typed memory object
};

#endif //_STRUCT_TYPED_MEM_INFO_H
