//elf64.c
//Executable loading
//Bryan E. Topp <betopp@betopp.com> 2021

#include "elf64.h"
#include "hal_uspc.h"
#include "hal_frame.h"
#include "kspace.h"
#include "kassert.h"
#include "libcstubs.h"

#include <fcntl.h>
#include <errno.h>

//ELF64 data types
typedef uint64_t elf64_addr_t; //Program address
typedef uint64_t elf64_off_t;  //File offset
typedef uint16_t elf64_half_t;
typedef uint32_t elf64_word_t;
typedef int32_t  elf64_sword_t;
typedef uint64_t elf64_xword_t;
typedef int64_t  elf64_sxword_t;

//Parts of ELF64 object identification
#define EI_MAG0 0 //Magic number, first byte
#define EI_MAG1 1 //Magic number, second byte
#define EI_MAG2 2 //Magic number, third byte
#define EI_MAG3 3 //Magic number, fourth byte
#define EI_CLASS 4 //File class
#define EI_DATA 5 //Endianness
#define EI_VERSION 6 //File format version
#define EI_OSABI 7 //OS and ABI ID
#define EI_ABIVERSION 8 //Version of OS and ABI
#define EI_PAD 9 //Start of padding bytes
#define EI_NIDENT 16 //Size of e_ident

//Possible values for e_ident[EI_CLASS]
#define ELFCLASS32 1
#define ELFCLASS64 2

//Possible values for e_ident[EI_DATA]
#define ELFDATA2LSB 1
#define ELFDATA2MSB 2

//Possible values for e_type
#define ET_NONE 0 //Invalid
#define ET_REL 1  //Relocatable
#define ET_EXEC 2 //Executable
#define ET_DYN 3  //Shared object

//ELF64 file header
typedef struct elf64_ehdr_s
{
	unsigned char e_ident[16]; //ELF identification
	elf64_half_t  e_type;      //Object file type
	elf64_half_t  e_machine;   //Machine type
	elf64_word_t  e_version;   //Object file version
	elf64_addr_t  e_entry;     //Entry point address
	elf64_off_t   e_phoff;     //Program headers offset
	elf64_off_t   e_shoff;     //Section headers offset
	elf64_word_t  e_flags;     //Processor-specific flags
	elf64_half_t  e_ehsize;    //ELF header size
	elf64_half_t  e_phentsize; //Size of each program header
	elf64_half_t  e_phnum;     //Number of program headers
	elf64_half_t  e_shentsize; //Size of each section header
	elf64_half_t  e_shnum;     //Number of section headers
	elf64_half_t  e_shstrndx;  //Index of section name string table
	
} elf64_ehdr_t;

//Possible values for p_type
#define PT_NULL    0 //Unused
#define PT_LOAD    1 //Loadable
#define PT_DYNAMIC 2 //Dynamic linking tables
#define PT_INTERP  3 //Interpreter path name
#define PT_NOTE    4 //Note sections
#define PT_SHLIB   5 //Reserved
#define PT_PHDR    6 //Program header table

//Program header flags, p_flags
#define PF_X 1
#define PF_W 2
#define PF_R 4


//ELF64 program header
typedef struct elf64_phdr_s
{
	elf64_word_t  p_type;   //Type of segment
	elf64_word_t  p_flags;  //Segment attributes
	elf64_off_t   p_offset; //Offset in file
	elf64_addr_t  p_vaddr;  //Virtual address in memory
	elf64_addr_t  p_paddr;  //Physical address in memory
	elf64_xword_t p_filesz; //Size of segment in file
	elf64_xword_t p_memsz;  //Size of segment in memory
	elf64_xword_t p_align;  //Alignment required of segment
	
} elf64_phdr_t;


int elf64_load(id_t fdid, mem_space_t *mem, uintptr_t *entry_out)
{		
	//HAL specifies granularity of memory we can load
	const size_t pagesize = hal_frame_size();
	
	//Buffers that will need to be freed on success or failure
	size_t phdr_buffer_size = 0;
	elf64_phdr_t *phdr_buffer = NULL;
	
	//Return value, whether we succeeded or not
	int retval = 0;
	
	//Read the ELF header from the beginning of the file.
	elf64_ehdr_t ehdr = {0};
	off_t seek_err = fd_seek(fdid, 0, SEEK_SET);
	if(seek_err < 0)
	{
		//Error seeking to beginning of file
		retval = seek_err;
		goto cleanup;
	}
	
	ssize_t header_read = fd_read(fdid, &ehdr, sizeof(ehdr));
	if(header_read < 0)
	{
		//Error reading header
		retval = header_read;
		goto cleanup;
	}
	
	if(header_read < (ssize_t)sizeof(ehdr))
	{
		//Short read of header
		retval = -ENOEXEC;
		goto cleanup;
	}
	
	//Validate magic
	if(ehdr.e_ident[EI_MAG0] != '\x7f' || ehdr.e_ident[EI_MAG1] != 'E' || ehdr.e_ident[EI_MAG2] != 'L' || ehdr.e_ident[EI_MAG3] != 'F')
	{
		//Bad magic number
		retval = -ENOEXEC;
		goto cleanup;
	}
	
	//Validate class of object file
	if(ehdr.e_ident[EI_CLASS] != ELFCLASS64)
	{
		//Wrong class
		retval = -ENOEXEC;
		goto cleanup;
	}
	
	//Validate object file type
	if(ehdr.e_type != ET_EXEC)
	{
		//Wrong type
		retval = -ENOEXEC;
		goto cleanup;
	}
	
	//Validate program header size
	if(ehdr.e_phentsize != sizeof(elf64_phdr_t))
	{
		//Bad program header size
		retval = -ENOEXEC;
		goto cleanup;
	}
	
	//Make sure we track enough memory segments to map each program header with some space left.
	if(ehdr.e_phnum + 3 > MEM_SEG_MAX)
	{
		retval = -ENOEXEC;
		goto cleanup;
	}
	
	//Try to allocate enough space for all the program headers in memory.
	//We want to load them all at once, so they can't change underneath us.
	//(We need to iterate them more than once.)
	phdr_buffer_size = ehdr.e_phentsize * ehdr.e_phnum;
	if(phdr_buffer_size > 1048576)
	{
		//Protect the kernel against an image that says it has an enormous amount of program headers.
		retval = -ENOEXEC;
		goto cleanup;
	}
	
	phdr_buffer = kspace_alloc(phdr_buffer_size, alignof(elf64_phdr_t));
	if(phdr_buffer == NULL)
	{
		//No space for reading program headers into memory
		retval = -ENOMEM;
		goto cleanup;
	}
	memset(phdr_buffer, 0, phdr_buffer_size);
		
	//Read all program headers into the program header buffer.
	//Sanity check them as they're being read.
	for(uint32_t pp = 0; pp < ehdr.e_phnum; pp++)
	{
		//Seek to the program header and read it into memory.		
		off_t phdr_seek_err = fd_seek(fdid, ehdr.e_phoff + (pp * ehdr.e_phentsize), SEEK_SET);
		if(phdr_seek_err < 0)
		{
			//Failed to seek to program header
			retval = phdr_seek_err;
			goto cleanup;
		}
		
		KASSERT(ehdr.e_phentsize == sizeof(phdr_buffer[0]));
		ssize_t phdr_read = fd_read(fdid, &(phdr_buffer[pp]), ehdr.e_phentsize);
		if(phdr_read < 0)
		{
			//Error reading program header
			retval = phdr_read;
			goto cleanup;
		}
		
		if(phdr_read < ehdr.e_phentsize)
		{
			//Short read of program header
			retval = -ENOEXEC;
			goto cleanup;
		}
		
		//Sanity-check the program header once we've read it.
		elf64_phdr_t *phdr = &(phdr_buffer[pp]);
		if(phdr->p_vaddr % pagesize != 0)
		{
			//Misaligned segments not supported
			retval = -ENOEXEC;
			goto cleanup;
		}
		
		if(phdr->p_memsz % pagesize != 0)
		{
			//Non-page-length segments not supported
			retval = -ENOEXEC;
			goto cleanup;
		}
		
		if(phdr->p_memsz < phdr->p_filesz)
		{
			//In-memory size is specified as smaller than the on-disk size
			retval = -ENOEXEC;
			goto cleanup;
		}
	}
	
	//Work through all the program headers, allocating and mapping memory for all loadable ranges.
	for(uint32_t pp = 0; pp < ehdr.e_phnum; pp++)
	{		
		elf64_phdr_t *phdr = &(phdr_buffer[pp]);
		if(phdr->p_type != PT_LOAD)
			continue;
		
		KASSERT(phdr->p_vaddr % pagesize == 0);
		KASSERT(phdr->p_memsz % pagesize == 0);
		int prot = 0;
		if(phdr->p_flags & PF_R)
			prot |= MEM_PROT_R;
		if(phdr->p_flags & PF_W)
			prot |= MEM_PROT_W;
		if(phdr->p_flags & PF_X)
			prot |= MEM_PROT_X;
		
		int map_result = mem_space_add(mem, phdr->p_vaddr, phdr->p_memsz, prot);
		if(map_result < 0)
		{
			retval = map_result;
			goto cleanup;
		}
	}
	
	//Work through all the program headers and load the data from the file.
	for(uint32_t pp = 0; pp < ehdr.e_phnum; pp++)
	{
		elf64_phdr_t *phdr = &(phdr_buffer[pp]);
		if(phdr->p_type != PT_LOAD)
			continue;
		
		//Load the initialized region from the data in the file
		off_t data_seek_err = fd_seek(fdid, phdr->p_offset, SEEK_SET);
		if(data_seek_err < 0)
		{
			//Failed to seek to segment data in file
			retval = data_seek_err;
			goto cleanup;
		}
		
		const hal_uspc_id_t old_uspc = hal_uspc_current();
		hal_uspc_activate(mem->uspc);
		
		KASSERT(phdr->p_memsz >= phdr->p_filesz);
		ssize_t data_read = fd_read(fdid, (void*)(phdr->p_vaddr), phdr->p_filesz);
		
		hal_uspc_activate(old_uspc);
		
		if(data_read < 0)
		{
			//Error reading segment data
			retval = data_read;
			goto cleanup;
		}
		
		if(data_read < (ssize_t)(phdr->p_filesz))
		{
			//Short read of segment data
			retval = -ENOEXEC;
			goto cleanup;
		}
	}
	
	//Success. Write-out the entry point for the ELF, and return 0.
	*entry_out = ehdr.e_entry;
	retval = 0;
	
cleanup:
	
	//Clean up temporary space holding program-headers.
	if(phdr_buffer != NULL)
	{
		kspace_free(phdr_buffer, phdr_buffer_size);
		phdr_buffer = NULL;
	}
	
	return retval;
}

