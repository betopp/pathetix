//pcmd.c
//Standard argument parsing
//Bryan E. Topp <betopp@betopp.com> 2021

#include <pcmd.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>

//Prints help and exits.
static void pcmd_help(const pcmd_t *cmd)
{
	fprintf(stderr, "%s ", cmd->desc);
	fprintf(stderr, "(%s)\n", cmd->title);
	fprintf(stderr, "Version:  %s\n", cmd->version);
	fprintf(stderr, "Built at: %s\n", cmd->date);
	fprintf(stderr, "Built by: %s\n", cmd->user);
	fprintf(stderr, "Options:\n");
	for(int oo = 0; cmd->opts[oo].letters != NULL; oo++)
	{
		const pcmd_opt_t *opt = &(cmd->opts[oo]);
		fprintf(stderr, "\t%s ( ", opt->name);
		if(opt->words != NULL)
		{
			for(int ww = 0; opt->words[ww] != NULL; ww++)
			{
				fprintf(stderr, "--%s ", opt->words[ww]);
			}
		}
		for(int ll = 0; opt->letters[ll] != '\0'; ll++)
		{
			fprintf(stderr, "-%c ", opt->letters[ll]);
		}
		fprintf(stderr, ")\n\t\t%s\n", opt->desc);
	}
}

//Finds an option given a word-long-name.
static const pcmd_opt_t *pcmd_find_word(const pcmd_t *cmd, const char *name)
{
	//Search all defined options. Options array is terminated by an entry with NULL letters string.
	for(int oo = 0; cmd->opts[oo].letters != NULL; oo++)
	{
		const pcmd_opt_t *opt = &(cmd->opts[oo]);
		if(opt->words == NULL)
		{
			//No long names for this option.
			continue;
		}
		
		//Search long-names for this option. Long names are terminated by a NULL pointer.
		for(int ww = 0; opt->words[ww] != NULL; ww++)
		{
			if(!strcmp(opt->words[ww], name))
			{
				//Found it.
				return opt;
			}
		}
	}
	
	//Not found
	return NULL;
}

//Finds an option given a letter (short) name.
static const pcmd_opt_t *pcmd_find_letter(const pcmd_t *cmd, char letter)
{
	//Search all defined options, terminated by one with a NULL letters string.
	for(int oo = 0; cmd->opts[oo].letters != NULL; oo++)
	{
		const pcmd_opt_t *opt = &(cmd->opts[oo]);
		
		//See if the given letter is one defined in this option.
		if(strchr(opt->letters, letter) != NULL)
		{
			//Found it.
			return opt;
		}
	}
	
	//Not found
	return NULL;
}

void pcmd_parse(const pcmd_t *cmd, int argc, char **argv)
{
	//Run through all options and default their outputs.
	for(int oo = 0; cmd->opts[oo].letters != NULL; oo++)
	{
		const pcmd_opt_t *opt = &(cmd->opts[oo]);
		
		if(opt->given != NULL)
			*(opt->given) = false;
		
		if(opt->valb != NULL)
			*(opt->valb) = false;
		
		if(opt->vali != NULL)
			*(opt->vali) = 0;
		
		if(opt->valp != NULL)
			*(opt->valp) = NULL;
	}
	
	//Check each command-line argument - see if it corresponds to a known option.
	for(int aa = 1; aa < argc; aa++)
	{
		//Ignore arguments that don't start with a "-". They're not options for us.
		if(argv[aa][0] != '-')
			continue;
		
		//Ignore single "-" as an argument - used to indicate stdin as a filename.
		if(argv[aa][1] == '\0')
			continue;
		
		//Check explicitly for "help"
		if(!strcmp(argv[aa], "--help"))
		{
			pcmd_help(cmd);
			exit(0);
		}
		
		//Alright, looks like this is an option. Is it a GNU-style long option?
		if(argv[aa][1] == '-')
		{
			//Long option. Name starts after the "--" and continues until a "=" if any.
			//Parameter value starts after the "=" if it exists.
			char *opt_name = &(argv[aa][2]);
			char *opt_eq = strchr(opt_name, '=');
			char *opt_parm = NULL;
			if(opt_eq != NULL)
			{
				//This arg contains an equal - and therefore a value.
				opt_parm = opt_eq + 1;
				
				//Terminate the name before the "=".
				*opt_eq = '\0';
			}
			
			//See if we know about this option.
			const pcmd_opt_t *opt = pcmd_find_word(cmd, opt_name);
			if(opt == NULL)
			{
				//Didn't find this option.
				fprintf(stderr, "%s: unknown option \"%s\".\n", argv[0], opt_name);
				exit(-1);
			}
			
			//Alright, the option was given.
			if(opt->given != NULL)
			{
				//Regardless of parameter, given = true now.
				*(opt->given) = true;
			}
			
			//If a value was given, output that
			if(opt_parm != NULL && opt_parm[0] != '\0')
			{
				//We've at least got the parameter string, so store where that is.
				if(opt->valp != NULL)
					*(opt->valp) = opt_parm;
				
				//Try to parse as a number.
				int val = atoi(opt_parm);
				
				//Handle special-cases for names that correspond to 0 or 1.
				static const char * yes_vals[] = { "y", "t", "yes", "true", NULL };
				for(int yy = 0; yes_vals[yy] != NULL; yy++)
				{
					if(!strcasecmp(opt_parm, yes_vals[yy]))
						val = 1;
				}
				
				static const char * no_vals[] = { "n", "f", "no", "false", NULL };
				for(int nn = 0; no_vals[nn] != NULL; nn++)
				{
					if(!strcasecmp(opt_parm, no_vals[nn]))
						val = 0;
				}
				
				//Output in appropriate integer/boolean locations.
				if(opt->vali != NULL)
					*(opt->vali) = val;
				
				if(opt->valb != NULL)
					*(opt->valb) = val ? true : false;
			}
			
			//Consumed the argument. Stick a NUL at the beginning to turn it into an empty-string.
			argv[aa][0] = '\0';
			continue;
		}
		else
		{
			//Not a long option. Parse short options, which may be stacked together in one argument.
			//Todo - how do we decide when a short option has a value associated with it?
			for(int ll = 1; argv[aa][ll] != '\0'; ll++)
			{
				const pcmd_opt_t *opt = pcmd_find_letter(cmd, argv[aa][ll]);
				if(opt == NULL)
				{
					//Didn't find this option.
					fprintf(stderr, "%s: unknown option \"%c\".\n", argv[0], argv[aa][ll]);
					exit(-1);
				}
				
				if(opt->given != NULL)
				{
					*(opt->given) = true;
				}
			}
			
			//Consumed the argument. Stick a NUL at the beginning to turn it into an empty-string.
			argv[aa][0] = '\0';
			continue;
		}
	}
	
	return;
}

