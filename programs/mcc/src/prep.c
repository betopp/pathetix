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


//Depth of "if" conditions, whether successful or failed
static int prep_if_level;

//Last "if" condition that was successful.
static int prep_if_match;

//Handles preprocessor directive - define
static void prep_d_define(tok_t *afterkey, tok_t *end)
{
	if(prep_if_match < prep_if_level)
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
	if(prep_if_match < prep_if_level)
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
	if(prep_if_match < prep_if_level)
	{
		//Higher-level condition has already failed.
		prep_if_level++;
		return;
	}
	
	//Should have at least one token here
	if(end->next == afterkey)
	{
		tok_err(afterkey, "expected expression");
	}
	
	//Copy the contents of the "if" our by itself
	tok_t *if_toks = tok_copy(afterkey, end);
	
	//Process tokens for macro replacement
	tok_t *mm = if_toks;
	while(mm != NULL)
	{
		tok_t *replaced_start = NULL;
		tok_t *replaced_end = NULL;
		tok_t *replaced_follow = NULL;
		macro_process(mm, &replaced_start, &replaced_end, &replaced_follow);		
		
		if(if_toks == mm)
		{
			//Processed the first token
			if(replaced_start == NULL)
			{
				//Deleted the first token
				if_toks = replaced_follow;
			}
			else
			{
				//Replaced the first token
				if_toks = replaced_start;
			}
		}
		
		mm = replaced_follow;
	}
	
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
	prep_if_level++;
	if(tinfo_val_nz(predicate->tinfo, predicate->value))
	{
		prep_if_match++;
	}
	
	syntax_free(predicate);
	tok_delete_all(if_toks_start);
}

//Handles preprocessor directive - ifdef
static void prep_d_ifdef(tok_t *afterkey, tok_t *end)
{
	if(afterkey->type != TOK_IDENT)
	{
		tok_err(afterkey, "expected identifier");
	}
	
	bool isdef = macro_isdef(afterkey->text);
	
	prep_if_level++;
	if(isdef)
		prep_if_match++;
}

//Handles preprocessor directive - ifndef
static void prep_d_ifndef(tok_t *afterkey, tok_t *end)
{
	if(afterkey->type != TOK_IDENT)
	{
		tok_err(afterkey, "expected identifier");
	}
	
	bool isdef = macro_isdef(afterkey->text);
	
	prep_if_level++;
	if(!isdef)
		prep_if_match++;
}

//Handles preprocessor directive - else
static void prep_d_else(tok_t *afterkey, tok_t *end)
{
	if(prep_if_match == prep_if_level - 1)
	{
		//Very last condition was the only one that didn't match.
		//We're now in the "else" side, so it does.
		prep_if_match = prep_if_level;
	}
	else if(prep_if_match == prep_if_level)
	{
		//All conditions were satisfied, and now the most recent one isn't.
		prep_if_match = prep_if_level - 1;
	}
	else
	{
		//A higher condition didn't match, so the "else" doesn't matter.
	}
}

//Handles preprocessor directive - elif
static void prep_d_elif(tok_t *afterkey, tok_t *end)
{
	prep_d_else(afterkey, end);
	prep_d_if(afterkey, end);
}

//Handles preprocessor directive - endif
static void prep_d_endif(tok_t *afterkey, tok_t *end)
{
	if(prep_if_match == prep_if_level)
		prep_if_match--;

	prep_if_level--;
	
	assert(prep_if_match <= prep_if_level);
}

//Handles preprocessor directive - error
static void prep_d_error(tok_t *afterkey, tok_t *end)
{
	if(prep_if_match < prep_if_level)
		return;
	
	tok_err(afterkey, "#error");
}

//Handles preprocessor directive - include
static void prep_d_include(tok_t *afterkey, tok_t *end)
{
	if(prep_if_match < prep_if_level)
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
	while(tt_start != NULL)
	{
		//Preprocessor directives must start at start-of-file or newline, and be followed by an octothorpe (#).
		bool at_newline = (tt_start->type == TOK_FILE) || (tt_start->type == TOK_NEWLINE);
		bool hash_next = (tt_start->next != NULL) && (tt_start->next->type == TOK_HASH);
		if(!(at_newline && hash_next))
		{
			//Not a preprocessor directive.
			if(prep_if_match < prep_if_level)
			{
				//If we're failing a preprocessor conditional (ifdef etc) then delete the token.
				tok_t *next = tt_start->next;
				tok_delete_single(tt_start);
				tt_start = next;
				continue;
			}
			else
			{
				//If the token passes preprocessor conditionals, do macro-replacement
				tok_t *replace_start = NULL;
				tok_t *replace_end = NULL;
				tok_t *replace_follow = NULL;
				macro_process(tt_start, &replace_start, &replace_end, &replace_follow);
				tt_start = replace_follow;
				continue;
			}
		}
		
		//Alright, found a line beginning with an octothorpe.
		
		//The preprocessor directive continues until the following newline.
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
		continue;
	}
}

void prep_repl(tok_t *tok)
{
	tok_pass_nums(tok); //Convert numbers
	tok_pass_keyw(tok); //Find keywords
	tok_pass_nowh(tok); //Delete whitespace
}
