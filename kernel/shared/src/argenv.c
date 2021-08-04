//argenv.c
//Argument and environment passing in kernel
//Bryan E. Topp <betopp@betopp.com> 2021

#include "argenv.h"
#include "kassert.h"
#include "libcstubs.h"
#include "kspace.h"

#include <errno.h>

int argenv_load(mem_space_t *mem, char * const * argv, char * const * envp)
{	
	//Todo - this needs to read from userspace safely (ugh).
	
	//Track how much size is needed.
	//We need to store all the NUL-terminated string literals.
	//Count the number of string literals and their total size.
	size_t nargv = 0;
	size_t nenvp = 0;
	size_t nstrings = 0;
	size_t nultermsize = 0;
	for(int ii = 0; argv[ii] != NULL; ii++)
	{
		nargv++;
		nstrings++;
		nultermsize += strlen(argv[ii]) + 1;
	}
	for(int ii = 0; envp[ii] != NULL; ii++)
	{
		nenvp++;
		nstrings++;
		nultermsize += strlen(envp[ii]) + 1;
	}
	
	//We need to store a pointer to each one, in an array of arg and env pointers.
	//We also need a NULL pointer to terminate the arg pointers and the env pointers.
	//Then we need a pointer to the beginning of each.
	size_t space_needed = nultermsize + ( (nstrings + 2) * sizeof(char*)) + (2 * sizeof(char**));
	
	//Round up to page size
	const size_t pagesize = hal_frame_size();
	space_needed = ((space_needed + pagesize - 1) / pagesize) * pagesize;
	
	//Sanity-check
	if(space_needed > 65536)
		return -E2BIG;
	
	//Build the buffer into kernel-space
	char *kbuf = kspace_alloc(space_needed, pagesize);
	if(kbuf == NULL)
		return -ENOMEM;
	

	//Layout will be:
	//Pointer to argpointers
	//Pointer to envpointers
	//Argpointers array
	//NULL
	//Envpointers array
	//NULL
	//String data
	
	//What a fucking mess - can we move this disgusting garbage to userspace?
	//Even after I come back and audit this I don't think I can make this safe.
	char *kbuf_next = kbuf;
	uintptr_t base = pagesize; //Where we'll map this in userspace (page +1)
	
	char ***argv_ptr_ptr = (char***)kbuf_next;
	kbuf_next += sizeof(char**);
	char ***envp_ptr_ptr = (char***)kbuf_next;
	kbuf_next += sizeof(char**);
	char **argv_array_ptr = (char**)kbuf_next;
	kbuf_next += sizeof(char*) * (nargv+1);
	char **envp_array_ptr = (char**)kbuf_next;
	kbuf_next += sizeof(char*) * (nenvp+1);
	char *string_data = (char*)kbuf_next;
	
	*argv_ptr_ptr = (char**)(base + (2 * sizeof(char**)));
	*envp_ptr_ptr = (char**)(base + (2 * sizeof(char**)) + ((nargv + 1) * sizeof(char*)));
	
	for(size_t aa = 0; aa < nargv; aa++)
	{
		argv_array_ptr[aa] = (char*)(base + (string_data - kbuf));
		
		size_t copylen = strlen(argv[aa]);
		
		KASSERT(copylen + (kbuf_next - kbuf) < space_needed);
		memcpy(string_data, argv[aa], copylen);
		
		string_data += copylen + 1;
	}
	
	for(size_t aa = 0; aa < nenvp; aa++)
	{
		envp_array_ptr[aa] = (char*)(base + (string_data - kbuf));
		
		size_t copylen = strlen(envp[aa]);
		
		KASSERT(copylen + (kbuf_next - kbuf) < space_needed);
		memcpy(string_data, envp[aa], copylen);
		
		string_data += copylen + 1;
	}
	
	//Make space to store this in the new userspace.
	int map_err = mem_space_add(mem, base, space_needed, MEM_PROT_R);
	if(map_err < 0)
	{
		//Failed to make space in the userspace for argv/envp data
		kspace_free(kbuf, space_needed);
		return map_err;
	}
	
	//Copy to that buffer in the new userspace
	hal_uspc_id_t old_uspc = hal_uspc_current();
	hal_uspc_activate(mem->uspc);
		
	memcpy((char*)base, kbuf, space_needed);
	
	hal_uspc_activate(old_uspc);
	
	//Success
	kspace_free(kbuf, space_needed);
	return 0;
}



