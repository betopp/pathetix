//dirs.c
//Directory searching for C compiler
//Bryan E. Topp <betopp@betopp.com> 2021

#include "dirs.h"
#include "alloc.h"

#include <stdlib.h>
#include <string.h>

typedef struct dir_list_s
{
	struct dir_list_s *next;
	const char *dir;
} dir_list_t;
dir_list_t *dir_lists[2] = 
{
	[DIRS_USR] = NULL,
	[DIRS_SYS] = NULL,
};

void dirs_add(int dirs, const char *path)
{
	//Make an entry and stick it on the head of the directory list
	dir_list_t *l = alloc_mandatory(sizeof(dir_list_t));
	l->dir = strdup_mandatory(path);	
	l->next = dir_lists[dirs];
	dir_lists[dirs] = l;
}

FILE *dirs_find(int dirs, const char *file)
{
	//Search the given directory list, trying to open the file in each directory
	for(dir_list_t *dd = dir_lists[dirs]; dd != NULL; dd = dd->next)
	{
		//Compute and allocate necessary size to store the whole path
		size_t dirlen = strlen(dd->dir);
		size_t fnlen = strlen(file);
		char *file_in_dir = alloc_mandatory(dirlen + 1 + fnlen + 1);
		
		//Concatenate the directory name and file name
		strcpy(file_in_dir, dd->dir);
		strcpy(file_in_dir + dirlen, "/");
		strcpy(file_in_dir + dirlen + 1, file);
		
		//Try to open the file
		FILE *fp = fopen(file_in_dir, "r");		
		
		//Free the filename buffer regardless of the result
		free(file_in_dir);
		
		//If we got the file open, return it
		if(fp != NULL)
		{
			return fp;
		}			
	}
	
	//Failed to open the file
	return NULL;
}
