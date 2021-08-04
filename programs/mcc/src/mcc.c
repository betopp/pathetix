//mcc.c
//Mediocre C compiler
//Bryan E. Topp <betopp@betopp.com> 2021

#include "tok.h"
#include "prep.h"
#include "dirs.h"
#include "alloc.h"

#include <stdlib.h>
#include <string.h>

int main(int argc, const char **argv)
{
	if(argc < 2)
	{
		printf("usage: %s input\n", argv[0]);
		exit(0);
	}
	
	//Add system include directories
	dirs_add(DIRS_SYS, "/usr/include");
	
	//Add the directory containing the input file to the search path
	char *lastslash = strrchr(argv[1], '/');
	if(lastslash == NULL)
	{
		//No slashes - file is in current directory
		dirs_add(DIRS_USR, ".");
	}
	else
	{
		//Path contains a slash - path up to the last slash is the directory
		size_t dirlen = lastslash - argv[1];
		char *dirstr = alloc_mandatory(dirlen + 1);
		
		memcpy(dirstr, argv[1], dirlen);
		dirstr[dirlen] = '\0';
		
		dirs_add(DIRS_USR, dirstr);
		
		free(dirstr);
	}
	
	//Open the main input file and tokenize it.
	FILE *fp = fopen(argv[1], "r");
	if(fp == NULL)
	{
		perror("fopen");
		return -1;
	}
	
	tok_t *fp_tok = tok_read(fp);
	fclose(fp);
	
	//Perform preprocessing passes on the file as a list of tokens
	prep_pass(fp_tok);
	
	//Print result for debugging for now
	for(tok_t *tt = fp_tok; tt != NULL; tt = tt->next)
	{
		//printf("%s\n\t%s\n", tok_typename(tt), (tt->text != NULL) ? tt->text : "");
		printf("%s ", tt->text);
	}
	
	return 0;
}
