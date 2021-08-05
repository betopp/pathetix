//macro.c
//Preprocessor macros
//Bryan E. Topp <betopp@betopp.com> 2021

#include "macro.h"
#include "alloc.h"

#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

//Macros defined
typedef struct macro_s
{
	struct macro_s *next; //Next macro in list of macros presently defined
	
	char *name; //Name of the macro, as NUL-terminated string
	bool funclike; //Whether this macro is function-like (true) or object-like (false)
	char **params; //Parameters of the macro, NULL-terminated array of NUL-terminated string pointers.
	size_t nparams; //Number of parameters the macro takes
	tok_t *toks; //Tokens that make up the body of the macro
	
	
	
} macro_t;
static macro_t *macro_list;

void macro_define(const char *name, tok_t *aftername)
{
	if(!strcmp(name, "defined"))
	{
		tok_err(aftername->prev, "cannot define \"defined\"");
	}
	
	//Undefine the macro if it was already defined as something else
	macro_undef(name);

	//Make room for the macro definition and store the name
	macro_t *mptr = alloc_mandatory(sizeof(macro_t)); 
	mptr->name = strdup_mandatory(name);
	
	//Parse tokens that occur after the name.
	//See if this is an object-like or function-like macro - does it have parameters?
	//Determine where the body of the macro begins.
	tok_t *body = NULL;
	mptr->params = alloc_mandatory(sizeof(const char*));
	mptr->params[0] = NULL;
	mptr->nparams = 0;
	if(aftername->type == TOK_PARENL && aftername->immediate)
	{
		//Name is followed immediately by a left paren (no spaces inbetween).
		mptr->funclike = true;
		tok_t *lparen = aftername;
		
		//That left-paren should be followed by a parameter list - which may be empty.
		if(lparen->next->type == TOK_PARENR)
		{
			//Empty list of parameters.
			body = lparen->next->next;
		}
		else if(lparen->next->type == TOK_IDENT)
		{
			//At least one parameter.
			//Figure out how many parameters we have and set aside all their names.
			tok_t *param = lparen->next;
			while(1)
			{
				//Should be looking at an identifier, the parameter
				if(param->type != TOK_IDENT)
				{
					tok_err(param, "expected identifier");
				}
				
				//Increment count of parameters, and make space in parameter array
				mptr->nparams++;
				mptr->params = realloc_mandatory(mptr->params, (mptr->nparams + 1) * sizeof(const char*));
				
				//Copy the parameter name
				mptr->params[mptr->nparams - 1] = strdup_mandatory(param->text);
				mptr->params[mptr->nparams] = NULL;
				
				//See what follows it
				if(param->next->type == TOK_PARENR)
				{
					//End of parameter list
					body = param->next->next;
					break;
				}
				else if(param->next->type == TOK_COMMA)
				{
					//Parameters separated by comma
					param = param->next->next;
					continue;
				}
				else
				{
					tok_err(param->next, "expected identifier or ,");
				}
			}
		}
		else
		{
			tok_err(lparen->next, "expected identifier or )");
		}
	}
	else
	{
		//Name isn't followed immediately by a left paren.
		//The body begins after the name.
		mptr->funclike = false;
		body = aftername;
	}
	
	//Alright, we've advanced past the parameters list, if any.
	//We're now looking at the body of the macro. The body runs until just before the next end-of-line.
	//The body is allowed to be empty as well - in which case we're looking right at the end-of-line.
	if(body->type != TOK_NEWLINE)
	{
		tok_t *copyend = body;
		while(copyend->type != TOK_NEWLINE)
		{
			if(copyend->type == TOK_EOF || copyend->type == TOK_FILE)
			{
				tok_err(copyend, "encountered end-of-file during macro definition");
			}
			copyend = copyend->next;
		}
		
		mptr->toks = tok_copy(body, copyend->prev);
	}
	
	//Put the macro in our list of defined macros.
	mptr->next = macro_list;
	macro_list = mptr;
}

void macro_undef(const char *name)
{
	if(!strcmp(name, "defined"))
	{
		fprintf(stderr, "cannot define \"defined\"");
		exit(-1);
	}
	
	//Find the macro in our list of macros
	macro_t **mmptr = &macro_list;
	for(macro_t *mm = macro_list; mm != NULL; mm = mm->next)
	{
		if(!strcmp(mm->name, name))
		{
			//Found the macro to remove.
			//Cut it from the list.
			*mmptr = mm->next;
			
			//Free it all
			for(int pp = 0; mm->params[pp] != NULL; pp++)
			{
				free(mm->params[pp]);
			}
			free(mm->params);
			free(mm->name);
			tok_delete_all(mm->toks);
			free(mm);
			
			//Shouldn't have duplicates - return after freeing one match.
			return;
		}
		
		mmptr = &(mm->next);
	}
	
	//Didn't find the macro, ignore undef.
}

bool macro_isdef(const char *name)
{
	for(macro_t *mm = macro_list; mm != NULL; mm = mm->next)
	{
		if(!strcmp(name, mm->name))
			return true;
	}
	
	return false;
}

tok_t *macro_process(tok_t *line)
{
	//Process macros on the line until none remain
	tok_t *line_pos = line;
	while(1)
	{
		//When we hit EOL or end of list, we're done
		if(line_pos == NULL || line_pos->type == TOK_NEWLINE || line_pos->type == TOK_EOF)
		{
			return line;
		}
		
		//Skip until identifier - only identifiers can start macros.
		if(line_pos->type != TOK_IDENT)
		{
			line_pos = line_pos->next;
			continue;
		}
	
		//See if this is a "defined" macro. This is a special case.
		if(!strcmp(line_pos->text, "defined"))
		{
			//Defined should always have one parameter
			if(line_pos->next->type != TOK_PARENL || line_pos->next->immediate == false)
			{
				tok_err(line_pos->next, "expected ( immediately after \"defined\"");
			}
			if(line_pos->next->next->type != TOK_IDENT)
			{
				tok_err(line_pos->next->next, "expected identifier");
			}
			if(line_pos->next->next->next->type != TOK_PARENR)
			{
				tok_err(line_pos->next->next->next, "expected )");
			}
			
			//Make a single replacement token - "0" or "1" depending on whether the operand is defined
			tok_t *result_tok = alloc_mandatory(sizeof(tok_t));
			result_tok->type = TOK_PNUMBER;
			result_tok->text = alloc_mandatory(2);
			result_tok->text[0] = macro_isdef(line_pos->next->next->text) ? '1' : '0';
			result_tok->text[1] = '\0';
			
			result_tok->nmacros = line_pos->nmacros + 1;
			result_tok->macros = alloc_mandatory(result_tok->nmacros * sizeof(char*));
			for(int mm = 0; mm < line_pos->nmacros; mm++)
			{
				result_tok->macros[mm] = strdup_mandatory(line_pos->macros[mm]);
			}
			result_tok->macros[line_pos->nmacros] = strdup_mandatory("defined");
			
			//Link in place of original four-token sequence
			result_tok->prev = line_pos->prev;
			if(result_tok->prev != NULL)
				result_tok->prev->next = result_tok;
			
			result_tok->next = line_pos->next->next->next->next;
			if(result_tok->next != NULL)
				result_tok->next->prev = result_tok;
			
			line_pos->next->next->next->next = NULL;
			line_pos->prev = NULL;
			tok_delete_range(line_pos, line_pos->next->next->next);
			
			//Start over after making the replacement (and note that we might have changed the first token)
			if(line_pos == line)
				line = result_tok;
			
			line_pos = line;			
			continue;
		}
		
		//Search for a macro that matches the identifier
		macro_t *found = NULL;
		for(macro_t *mm = macro_list; mm != NULL; mm = mm->next)
		{
			if(!strcmp(mm->name, line_pos->text))
			{
				found = mm;
				break;
			}
		}
		
		if(found == NULL)
		{
			//No macro matches the identifier. Don't replace.
			line_pos = line_pos->next;
			continue;
		}
		
		//See if the macro was already used in creating this identifier
		bool recursed = false;
		for(size_t mm = 0; mm < line_pos->nmacros; mm++)
		{
			if(!strcmp(line_pos->macros[mm], line_pos->text))
			{
				recursed = true;
				break;
			}
		}
		
		if(recursed)
		{
			//Recursive use of this same macro. Don't replace.
			line_pos = line_pos->next;
			continue;
		}
		
		//Alright, this is a macro usage.
		
		//Find the range of tokens removed when the macro usage is replaced...
		tok_t *src_start = line_pos;
		tok_t *src_end = line_pos;
		
		//Build list of tokens to insert in place of the replaced one...
		tok_t *repl_start = NULL;
		tok_t *repl_end = NULL;
		
		//Replacement depends on whether this is an object-like or function-like macro.
		if(!found->funclike)
		{
			//Macro doesn't take a parameter list.
			//Ignore whether there's a parameter list and just replace the one token.		
			//Copy macro definition straight, with no parameter replacement.
			if(found->toks != NULL)
			{
				repl_start = tok_copy_all(found->toks);
				repl_end = repl_start;
				while(repl_end->next != NULL)
				{
					repl_end = repl_end->next;
				}
			}
		}
		else
		{
			//Macro is defined as function-like.
			//Should be followed by a parameter list.
			if(line_pos->next == NULL || line_pos->next->type != TOK_PARENL || line_pos->next->immediate == false)
			{
				tok_err(line_pos, "macro defined as function-like but used without parameter list");
			}
			
			//Need to see the right number of parameters
			tok_t **parmvals = alloc_mandatory(sizeof(tok_t*) * (found->nparams + 1));
			size_t parmnext = 0;
			
			//Parameters start after left-paren
			src_end = line_pos->next->next;
			while(1)
			{
				//If we have no more parameters, we should see a right-paren.
				//Leave src_end pointing at the right-paren.
				if(parmnext >= found->nparams)
				{
					if(src_end->type != TOK_PARENR)
					{
						tok_err(src_end, "expected ) after parameter");
					}
					
					break;
				}
				
				//Otherwise, we should see an identifier for the parameter.
				if(src_end->type != TOK_IDENT && src_end->type != TOK_PNUMBER && src_end->type != TOK_STRLIT)
				{
					tok_err(src_end, "expected macro parameter");
				}
				
				parmvals[parmnext] = src_end;
				parmnext++;
				src_end = src_end->next;
				
				//Nonfinal parameter should be followed by comma
				if(parmnext < found->nparams)
				{
					if(src_end->type != TOK_COMMA)
					{
						tok_err(src_end, "expected , after macro parameter");
					}
					
					src_end = src_end->next;
				}
			}
			
			//Make replacement tokens, substituting parameter values.
			//Work through each token from the macro's definition.
			for(tok_t *rr = found->toks; rr != NULL; rr = rr->next)
			{
				//See if this token, in the macro definition, refers to a parameter
				int subidx = -1;
				for(int pp = 0; pp < found->nparams; pp++)
				{
					if(!strcmp(found->params[pp], rr->text))
					{
						subidx = pp;
						break;
					}
				}
				
				//Make the new token for the replacement token sequence
				tok_t *copied = NULL;
				if(subidx == -1)
				{
					//This token from the macro definition isn't a parameter.
					//Copy as-is.
					copied = tok_copy(rr, rr);
				}
				else
				{
					//This token from the macro definition is replaced by a parameter.
					//Replace with the parameter's value.
					copied = tok_copy(parmvals[subidx], parmvals[subidx]);
				}
				
				//Append to the replacement sequence
				if(repl_start == NULL)
				{
					repl_start = copied;
					repl_end = copied;
				}
				else
				{
					repl_end->next = copied;
					copied->prev = repl_end;
					
					repl_end = copied;
				}
			}
			
			free(parmvals);
		}
		
		//Put replacement into the source list, in place of the replaced macro.
		//Replacement might be empty (no tokens)
		if(repl_start != NULL)
		{
			repl_start->prev = src_start->prev;
			if(repl_start->prev != NULL)
				repl_start->prev->next = repl_start;
			
			repl_end->next = src_end->next;
			if(repl_end->next != NULL)
				repl_end->next->prev = repl_end;
			
			//Note the additional macro used in creation of those replacement tokens.
			//This is used to track whether a macro is being invoked recursively (and avoid it).
			//This could also be used for error reporting at some point...
			for(tok_t *rr = repl_start; rr != repl_end->next; rr = rr->next)
			{
				//Free old macro list for the token (came from macro definition/parameter)
				if(rr->macros != NULL)
				{
					for(size_t mm = 0; mm < rr->nmacros; mm++)
					{
						free(rr->macros[mm]);
					}
					free(rr->macros);
				}
				
				//List all macros that resulted in the macro being invoked - and that macro's name.
				rr->nmacros = src_start->nmacros + 1;
				rr->macros = alloc_mandatory(rr->nmacros * sizeof(char*));
				for(size_t mm = 0; mm < src_start->nmacros; mm++)
				{
					rr->macros[mm] = strdup_mandatory(src_start->macros[mm]);
				}
				rr->macros[src_start->nmacros] = strdup_mandatory(found->name);
			}
		}
		else
		{
			src_start->prev->next = src_start->next;
			src_start->next->prev = src_start->prev;
		}
		
		//Delete the replaced macro
		src_start->prev = NULL;
		src_end->next = NULL;
		tok_delete_range(src_start, src_end);
		
		//Start the line over (and note that we might have replaced the first token in the line)
		if(line == src_start)
			line = repl_start;
		
		line_pos = line;
		continue;
	}
	
	assert(0);
}

