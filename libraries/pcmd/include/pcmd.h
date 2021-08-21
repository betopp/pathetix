//pcmd.h
//Standard argument parsing
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef PCMD_H
#define PCMD_H

#include <stdbool.h>

//Definition of one possible command-line option
typedef struct pcmd_opt_s
{
	const char * name; //Name of this option in help-text
	const char * desc; //Description of this option in help-text
	
	const char * letters; //Single-character names for the option
	const char * const * words; //Word-long names for the option
	
	bool *given; //Where to store whether the option was given
	
	char **valp; //Where to store the location of the value as a string pointer (NULL if no value)
	bool *valb; //Where to store the value as a boolean (false if no value)
	int *vali; //Where to store the value as an integer (0 if no value)
	
} pcmd_opt_t;

//Const data structure that defines possible options for the program.
typedef struct pcmd_s
{
	const char *title; //Title of the program
	const char *desc; //Description of what it does
	
	const char *version; //Build version
	const char *date; //Build date
	const char *user; //Build user
	
	const pcmd_opt_t *opts; //Possible command-line options
	
} pcmd_t;

//Parses command-line arguments and fills results.
//May print-and-exit if help is requested.
//Consumes arguments that are parsed; they will be replaced with empty-strings.
void pcmd_parse(const pcmd_t *cmd, int argc, char **argv);

#endif //PCMD_H
