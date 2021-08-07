//prep.c
//Preprocessor pass for C compiler
//Bryan E. Topp <betopp@betopp.com> 2021

#include "prep.h"
#include "dirs.h"
#include "macro.h"
#include "syntax.h"
#include "tinfo.h"
#include "alloc.h"

#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

//Depth of nested "if" blocks
int prep_if_depth;

//Results of "if" calls
#define PREP_IF_MAX 128
#define PREP_IF_STATE_PASS (1) //Successful condition
#define PREP_IF_STATE_FAIL (2) //Failed condition
#define PREP_IF_STATE_ELSE (3) //Failed because we already passed the successful block and had an else/elif
int prep_if_state[PREP_IF_MAX];

//Whether conditionals are currently satisfied.
bool prep_if_pass = true;

//Recomputes prep_if_pass
void prep_if_pass_compute(void)
{
	prep_if_pass = true;
	for(int dd = 0; dd < prep_if_depth; dd++)
	{
		switch(prep_if_state[dd])
		{
			case PREP_IF_STATE_PASS:
				break;
			
			case PREP_IF_STATE_FAIL:
				prep_if_pass = false;
				return;
			
			case PREP_IF_STATE_ELSE:
				prep_if_pass = false;
				return;
		
			default: abort();
		}
	}
}

//Handles preprocessor directive - define
static void prep_d_define(tok_t *afterkey, tok_t *end)
{	
	if(!prep_if_pass)
		return;
	
	//Define should be followed by an identifier for the name of the macro
	if(afterkey->type != TOK_IDENT)
	{
		tok_err(afterkey, "expected identifier");
	}
	
	macro_define(afterkey->text, afterkey->next);
}

//Handles preprocessor directive - undef
static void prep_d_undef(tok_t *afterkey, tok_t *end)
{	
	if(!prep_if_pass)
		return;
	
	//Undef should be followed by an identifier for the name of the macro
	if(afterkey->type != TOK_IDENT)
	{
		tok_err(afterkey, "expected identifier");
	}
	
	//That should be the only thing in this preprocessor directive
	if(afterkey != end)
	{
		tok_err(afterkey->next, "expected end-of-line");
	}
	
	macro_undef(afterkey->text);
}

//Handles preprocessor directive - if
static void prep_d_if(tok_t *afterkey, tok_t *end)
{
	//Limit nesting depth
	if(prep_if_depth >= PREP_IF_MAX)
	{
		tok_err(afterkey->prev, "nested too deeply");
	}
	
	//Should have at least one token here
	if(end->next == afterkey)
	{
		tok_err(afterkey, "expected expression");
	}
	
	//Copy the contents of the "if" our by itself
	tok_t *if_toks = tok_copy(afterkey, end);
	
	//Process tokens for macro replacement
	if_toks = macro_process(if_toks);

	//Any remaining identifiers are converted to preprocessor number 0
	for(tok_t *rr = if_toks; rr != NULL; rr = rr->next)
	{
		if(rr->type == TOK_IDENT)
		{
			rr->type = TOK_PNUMBER;
			free(rr->text);
			rr->text = alloc_mandatory(2);
			rr->text[0] = '0';
			rr->text[1] = '\0';
		}
	}
	
	//Convert to language tokens
	prep_repl(if_toks);
	
	//Try to make an expression of it
	tok_t *if_toks_start = if_toks;
	tok_t *if_toks_end = if_toks_start;
	while(if_toks_end->next != NULL)
	{
		if_toks_end = if_toks_end->next;
	}
	syntax_node_t *predicate = syntax_try(S_CONSTANT_EXPRESSION, if_toks_start, if_toks_end);
	if(predicate == NULL)
	{
		tok_err(afterkey, "expected constant-expression");
	}
	
	//Do constant-evaluation on that expression and see if it comes out const or not
	syntax_doconst(predicate);
	if(!predicate->value)
	{
		tok_err(afterkey, "not compile-time constant");
	}
	
	//Condition based on whether the expression is nonzero
	prep_if_state[prep_if_depth] = tinfo_val_nz(predicate->tinfo, predicate->value) ? PREP_IF_STATE_PASS : PREP_IF_STATE_FAIL;
	prep_if_depth++;
	prep_if_pass_compute();
	
	syntax_free(predicate);
	tok_delete_all(if_toks_start);
}

//Handles preprocessor directive - ifdef
static void prep_d_ifdef(tok_t *afterkey, tok_t *end)
{
	//Limit nesting depth
	if(prep_if_depth >= PREP_IF_MAX)
	{
		tok_err(afterkey->prev, "nested too deeply");
	}
	
	if(afterkey->type != TOK_IDENT)
	{
		tok_err(afterkey, "expected identifier");
	}
	
	bool isdef = macro_isdef(afterkey->text);
	
	prep_if_state[prep_if_depth] = isdef ? PREP_IF_STATE_PASS : PREP_IF_STATE_FAIL;
	prep_if_depth++;
	prep_if_pass_compute();
}

//Handles preprocessor directive - ifndef
static void prep_d_ifndef(tok_t *afterkey, tok_t *end)
{
	//Limit nesting depth
	if(prep_if_depth >= PREP_IF_MAX)
	{
		tok_err(afterkey->prev, "nested too deeply");
	}
	
	if(afterkey->type != TOK_IDENT)
	{
		tok_err(afterkey, "expected identifier");
	}
	
	bool isdef = macro_isdef(afterkey->text);
	
	prep_if_state[prep_if_depth] = (!isdef) ? PREP_IF_STATE_PASS : PREP_IF_STATE_FAIL;
	prep_if_depth++;
	prep_if_pass_compute();
}

//Handles preprocessor directive - else
static void prep_d_else(tok_t *afterkey, tok_t *end)
{
	if(prep_if_depth == 0)
		tok_err(afterkey->prev, "expected if before else");
	
	//An "else" doesn't change the level of nesting, but alters whether we passed/failed
	int last = prep_if_depth-1;
	switch(prep_if_state[last])
	{
		case PREP_IF_STATE_PASS:
			//Passed before. Now we're in the failing side.
			prep_if_state[last] = PREP_IF_STATE_ELSE;
			break;
		case PREP_IF_STATE_FAIL:
			//Failed before. Now we're in the passing side.
			prep_if_state[last] = PREP_IF_STATE_PASS;
			break;
		case PREP_IF_STATE_ELSE:
			//Further failure case of an elif sequence where some other case succeeded.
			//We're still not in the passing side.
			prep_if_state[last] = PREP_IF_STATE_ELSE;
			break;
		default: abort();
	}
	
	prep_if_pass_compute();
}

//Handles preprocessor directive - elif
static void prep_d_elif(tok_t *afterkey, tok_t *end)
{	
	if(prep_if_depth == 0)
		tok_err(afterkey->prev, "expected if before elif");
	
	//Evaluate the "if" statement just encountered
	prep_d_if(afterkey, end);
	
	//Pop its result off the stack
	int new_result = prep_if_state[prep_if_depth-1];
	prep_if_state[prep_if_depth-1] = 0;
	prep_if_depth--;
	
	//Combine with the existing "elif" sequence
	int last = prep_if_depth-1;
	switch(prep_if_state[last])
	{
		case PREP_IF_STATE_PASS:
			//We were previously in a passing block of the elif sequence.
			//So we never enter this one, or any further elifs, regardless of the "if" result.
			prep_if_state[last] = PREP_IF_STATE_ELSE;
			break;
		case PREP_IF_STATE_FAIL:
			//We were previously in a failed block of the elif sequence.
			//If this condition succeeded, we enter it. Otherwise, continue being in a failed state.
			if(new_result == PREP_IF_STATE_PASS)
				prep_if_state[last] = PREP_IF_STATE_PASS;
			
			break;
		case PREP_IF_STATE_ELSE:
			//We had already satisfied, and moved past, some other block of the elif sequence.
			//We never go back. Stay in "other block satisfied" state.
			prep_if_state[last] = PREP_IF_STATE_ELSE;
			break;
		default: abort();
	}
	
	prep_if_pass_compute();
}

//Handles preprocessor directive - endif
static void prep_d_endif(tok_t *afterkey, tok_t *end)
{
	if(prep_if_depth == 0)
		tok_err(afterkey->prev, "expected if before endif");
	
	prep_if_state[prep_if_depth-1] = 0;
	prep_if_depth--;
	
	prep_if_pass_compute();
}

//Handles preprocessor directive - error
static void prep_d_error(tok_t *afterkey, tok_t *end)
{
	if(!prep_if_pass)
		return;
	
	tok_err(afterkey, "#error");
}

//Handles preprocessor directive - include
static void prep_d_include(tok_t *afterkey, tok_t *end)
{
	if(!prep_if_pass)
		return;
	
	//Include directive should only have a single token after the keyword - a string
	if(afterkey->type != TOK_STRLIT && afterkey->type != TOK_SYSHDR )
	{
		tok_err(afterkey, "expected string or angle-bracket string");
	}
		
	if(afterkey != end)
	{
		tok_err(afterkey->next, "expected end-of-line");
	}
	
	printf("processing include %s\n", afterkey->text);
	
	//Erase the ending quote from the string and try to open the file skipping the opening quote
	afterkey->text[strlen(afterkey->text)-1] = '\0';
	int dirs = (afterkey->text[0] == '<') ? DIRS_SYS : DIRS_USR;
	FILE *included = dirs_find(dirs, afterkey->text + 1);
	if(included == NULL)
	{
		tok_err(afterkey, "failed to find file");
	}
	
	//If we got the file open, tokenize the included file
	tok_t *included_tok = tok_read(included);
	
	//Don't need the file after tokenizing it
	fclose(included);
	
	//Put the contents of the tokenized file after this line
	tok_t *ii_begin = included_tok;
	tok_t *ii_end = ii_begin;
	while(ii_end->next != NULL)
	{
		ii_end = ii_end->next;
	}
	
	ii_end->next = end->next;
	end->next->prev = ii_end;
	
	ii_begin->prev = end;
	end->next = ii_begin;
	
	//Caller deletes the preprocessor line.
}


//Table of preprocessor directives we recognize, and the functions to call for each.
typedef struct prep_dir_s
{
	const char *str;
	void (*func)(tok_t *afterkey, tok_t *end);
} prep_dir_t;
static const prep_dir_t prep_dir_array[] = 
{
	{ .str = "include", .func = &prep_d_include },
	{ .str = "define",  .func = &prep_d_define  },
	{ .str = "undef",   .func = &prep_d_undef   },
	{ .str = "if",      .func = &prep_d_if      },
	{ .str = "ifdef",   .func = &prep_d_ifdef   },
	{ .str = "ifndef",  .func = &prep_d_ifndef  },
	{ .str = "else",    .func = &prep_d_else    },
	{ .str = "elif",    .func = &prep_d_elif    },
	{ .str = "endif",   .func = &prep_d_endif   },
	{ .str = "error",   .func = &prep_d_error   },
	{ .str = NULL,      .func = NULL            }	
};

//Handles a single preprocessor directive and deletes it.
static void prep_line(tok_t *start, tok_t *end)
{
	//Skip over octothorpe and make sure it's followed by an identifier
	tok_t *keyword = start->next;
	if(keyword->type != TOK_IDENT)
	{
		tok_err(keyword, "expected identifier");
	}
	
	//Based on the directive, call the appropriate function to process it
	bool processed = false;
	for(int ii = 0; prep_dir_array[ii].str != NULL; ii++)
	{
		if(!strcmp(keyword->text, prep_dir_array[ii].str))
		{
			//Match - this is the directive to process
			(*(prep_dir_array[ii].func))(keyword->next, end);
			processed = true;
			break;
		}
	}
	
	if(!processed)
	{
		tok_err(keyword, "unhandled directive");
	}
	
	//Once the directive has been processed, delete it.
	tok_delete_range(start, end);
}

void prep_pass(tok_t *tok)
{
	//Run through the file and evaluate all preprocessor directives.
	//Directives begin with newline or start-of-file, then an octothorpe, then the rest of the line.
	tok_t *tt_start = tok;
	while(1)
	{		
		assert(tt_start != NULL && tt_start->type != TOK_NONE);
		
		//Advance past newlines/etc
		if(tt_start->type == TOK_FILE || tt_start->type == TOK_NEWLINE || tt_start->type == TOK_EOF)
		{
			if(tt_start->next == NULL)
			{
				//This is the only place we should actually get to end-of-list - advancing past EOF.
				assert(tt_start->type == TOK_EOF);
				break;
			}
			
			tt_start = tt_start->next;
			assert(tt_start != NULL && tt_start->type != TOK_NONE);
			continue;
		}
		
		//See if the line begins with an octothorpe, and is thus a preprocessor directive.
		if(tt_start->type == TOK_HASH)
		{
			//The preprocessor directive continues until the following newline.
			tt_start = tt_start->prev;
			assert(tt_start != NULL && tt_start->type != TOK_NONE);
			
			tok_t *tt_end = tt_start->next;
			while(tt_end->type != TOK_NEWLINE && tt_end->type != EOF)
			{
				tt_end = tt_end->next;
			}
			
			//tt_start now refers to a newline (or start-of-file) that begins the directive.
			//tt_end refers to the newline (or end-of-file) that ends the directive.
			
			//Process the stuff inbetween as a directive.
			prep_line(tt_start->next, tt_end->prev);
			
			//The directive will be deleted, leaving tt_start and tt_end (the newlines) untouched.
			//Optionally, new tokens will be inserted in its place.
			//Continue preprocessing from the same newline that once started the directive.
			assert(tt_start != NULL && tt_start->type != TOK_NONE);
			continue;
		}
		else
		{
			//Not a preprocessor directive.
			if(!prep_if_pass)
			{
				//If we're failing a preprocessor conditional (ifdef etc) then delete the line.
				
				//tt_start points to the contents after the newline - find where the next newline is
				tok_t *delete_start = tt_start;
				tok_t *delete_end = tt_start;
				while(delete_end->type != TOK_FILE && delete_end->type != TOK_NEWLINE && delete_end->type != TOK_EOF)
				{
					delete_end = delete_end->next;
				}
				
				//Back up to the token before this line, and delete the line and try again.
				tt_start = tt_start->prev;
				tok_delete_range(delete_start, delete_end);
				
				assert(tt_start != NULL && tt_start->type != TOK_NONE);
				continue;
			}
			else
			{
				//Run the line through macro replacement.
				tt_start = macro_process(tt_start);
				assert(tt_start != NULL && tt_start->type != TOK_NONE);
				
				//Advance to the end-of-line of the line we just processed.
				while(tt_start->type != TOK_FILE && tt_start->type != TOK_NEWLINE && tt_start->type != TOK_EOF)
				{
					if(tt_start->next == NULL)
						break;
					
					tt_start = tt_start->next;
					assert(tt_start != NULL && tt_start->type != TOK_NONE);
				}
				
				continue;
			}
		}
	}
	
	//Clean up tokens and turn preprocessor tokens into processing tokens
	prep_repl(tok);
}

void prep_repl(tok_t *tok)
{
	tok_pass_nums(tok); //Convert numbers
	tok_pass_keyw(tok); //Find keywords
	tok_pass_nowh(tok); //Delete whitespace
}
