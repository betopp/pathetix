//tok.c
//Tokenizer
//Bryan E. Topp <betopp@betopp.com> 2021

#include "tok.h"
#include "charin.h"
#include "alloc.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>


//Table of printable names for each token type
const char *tok_typenames[TOK_MAX] =
{
	[TOK_NONE] = "Invalid",
	[TOK_FILE] = "Start-of-file",
	[TOK_EOF] = "End-of-file",
	[TOK_NEWLINE] = "Newline",
	[TOK_IDENT] = "Identifier",
	[TOK_STRLIT] = "String-literal",
	[TOK_SYSHDR] = "Header-name",
	[TOK_CHARACTER] = "Character-constant",
	[TOK_PNUMBER] = "Number",
	[TOK_JUNK] = "Junk",
	[TOK_INTC] = "Integer-constant",
	[TOK_FLTC] = "Float-constant",
	[TOK_ELLIPS] = "...",
	[TOK_DLEQ] = "<<=",
	[TOK_DGEQ] = ">>=",
	[TOK_DLT] = "<<",
	[TOK_DGT] = ">>",
	[TOK_DPLUS] = "++",
	[TOK_DMINUS] = "--",
	[TOK_PLUSEQ] = "+=",
	[TOK_MINEQ] = "-=",
	[TOK_SLSHEQ] = "/=",
	[TOK_ASTEQ] = "*=",
	[TOK_EXCEQ] = "!=",
	[TOK_BAREQ] = "|=",
	[TOK_AMPEQ] = "&=",
	[TOK_CAREQ] = "^=",
	[TOK_PCTEQ] = "%=",
	[TOK_DEQ] = "==",
	[TOK_LEQ] = "<=",
	[TOK_GEQ] = ">=",
	[TOK_DBAR] = "||",
	[TOK_DAMP] = "&&",
	[TOK_ARROW] = "->",
	[TOK_DHASH] = "##",
	[TOK_LT] = "<",
	[TOK_GT] = ">",
	[TOK_HASH] = "#",
	[TOK_COMMA] = ",",
	[TOK_SCOLON] = ";",
	[TOK_PLUS] = "+",
	[TOK_MINUS] = "-",
	[TOK_SLASH] = "/",
	[TOK_ASTER] = "*",
	[TOK_PCT] = "%",
	[TOK_EQU] = "=",
	[TOK_BRACKL] = "[",
	[TOK_BRACKR] = "]",
	[TOK_PARENL] = "(",
	[TOK_PARENR] = ")",
	[TOK_BRACEL] = "{",
	[TOK_BRACER] = "}",
	[TOK_DOT] = ".",
	[TOK_EXCL] = "!",
	[TOK_TILDE] = "~",
	[TOK_BAR] = "|",
	[TOK_CARAT] = "^",
	[TOK_AMP] = "&",
	[TOK_QSTN] = "?",
	[TOK_COLON] = ":",
	[TOK_AUTO] = "auto",
	[TOK_BREAK] = "break",
	[TOK_CASE] = "case",
	[TOK_CHAR] = "char",
	[TOK_CONST] = "const",
	[TOK_CONTINUE] = "continue",
	[TOK_DEFAULT] = "default",
	[TOK_DO] = "do",
	[TOK_DOUBLE] = "double",
	[TOK_ELSE] = "else",
	[TOK_ENUM] = "enum",
	[TOK_EXTERN] = "extern",
	[TOK_FLOAT] = "float",
	[TOK_FOR] = "for",
	[TOK_GOTO] = "goto",
	[TOK_IF] = "if",
	[TOK_INLINE] = "inline",
	[TOK_INT] = "int",
	[TOK_LONG] = "long",
	[TOK_REGISTER] = "register",
	[TOK_RESTRICT] = "restrict",
	[TOK_RETURN] = "return",
	[TOK_SHORT] = "short",
	[TOK_SIGNED] = "signed",
	[TOK_SIZEOF] = "sizeof",
	[TOK_STATIC] = "static",
	[TOK_STRUCT] = "struct",
	[TOK_SWITCH] = "switch",
	[TOK_TYPEDEF] = "typedef",
	[TOK_UNION] = "union",
	[TOK_UNSIGNED] = "unsigned",
	[TOK_VOID] = "void",
	[TOK_VOLATILE] = "volatile",
	[TOK_WHILE] = "while",
	[TOK_BOOL] = "_Bool",
	[TOK_COMPLEX] = "_Complex",
	[TOK_IMAGINARY] = "_Imaginary",
	
};

//Table of tokens that consist of a given, fixed sequence of characters.
//NOTE - LONGER SEQUENCES FIRST! We simply check in order.
typedef struct tok_punct_s
{
	const char *intext;
	tok_type_t toktype;
} tok_punct_t;
static const tok_punct_t tok_punct_array[] = 
{
	//Digraphs
	{ .intext = "%:%:", .toktype = TOK_DHASH  },
	{ .intext = "%:",   .toktype = TOK_HASH   },
	{ .intext = "<%",   .toktype = TOK_BRACEL },
	{ .intext = "%>",   .toktype = TOK_BRACER },
	{ .intext = "<:",   .toktype = TOK_BRACKL },
	{ .intext = ":>",   .toktype = TOK_BRACKR },
	
	//Three-character punctuator
	{ .intext = "...",  .toktype = TOK_ELLIPS },
	{ .intext = "<<=",  .toktype = TOK_DLEQ   },
	{ .intext = ">>=",  .toktype = TOK_DGEQ   },
	
	//Two-character punctuators
	{ .intext = "<<",   .toktype = TOK_DLT    },
	{ .intext = ">>",   .toktype = TOK_DGT    },
	{ .intext = "++",   .toktype = TOK_DPLUS  },
	{ .intext = "--",   .toktype = TOK_DMINUS },
	{ .intext = "+=",   .toktype = TOK_PLUSEQ },
	{ .intext = "-=",   .toktype = TOK_MINEQ  },
	{ .intext = "/=",   .toktype = TOK_SLSHEQ },
	{ .intext = "*=",   .toktype = TOK_ASTEQ  },
	{ .intext = "!=",   .toktype = TOK_EXCEQ  },
	{ .intext = "|=",   .toktype = TOK_BAREQ  },
	{ .intext = "&=",   .toktype = TOK_AMPEQ  },
	{ .intext = "^=",   .toktype = TOK_CAREQ  },
	{ .intext = "%=",   .toktype = TOK_PCTEQ  },
	{ .intext = "==",   .toktype = TOK_DEQ    },
	{ .intext = "<=",   .toktype = TOK_LEQ    },
	{ .intext = ">=",   .toktype = TOK_GEQ    },
	{ .intext = "||",   .toktype = TOK_DBAR   },
	{ .intext = "&&",   .toktype = TOK_DAMP   },
	{ .intext = "->",   .toktype = TOK_ARROW  },
	{ .intext = "##",   .toktype = TOK_DHASH  },
	
	//One-character punctuators
	{ .intext = "<",    .toktype = TOK_LT     },
	{ .intext = ">",    .toktype = TOK_GT     },
	{ .intext = "#",    .toktype = TOK_HASH   },
	{ .intext = ",",    .toktype = TOK_COMMA  },
	{ .intext = ";",    .toktype = TOK_SCOLON },
	{ .intext = "+",    .toktype = TOK_PLUS   },
	{ .intext = "-",    .toktype = TOK_MINUS  },
	{ .intext = "/",    .toktype = TOK_SLASH  },
	{ .intext = "*",    .toktype = TOK_ASTER  },
	{ .intext = "%",    .toktype = TOK_PCT    },
	{ .intext = "=",    .toktype = TOK_EQU    },
	{ .intext = "[",    .toktype = TOK_BRACKL },
	{ .intext = "]",    .toktype = TOK_BRACKR },
	{ .intext = "(",    .toktype = TOK_PARENL },
	{ .intext = ")",    .toktype = TOK_PARENR },
	{ .intext = "{",    .toktype = TOK_BRACEL },
	{ .intext = "}",    .toktype = TOK_BRACER },
	{ .intext = ".",    .toktype = TOK_DOT    },
	{ .intext = "!",    .toktype = TOK_EXCL   },
	{ .intext = "~",    .toktype = TOK_TILDE  },
	{ .intext = "|",    .toktype = TOK_BAR    },
	{ .intext = "^",    .toktype = TOK_CARAT  },
	{ .intext = "&",    .toktype = TOK_AMP    },
	{ .intext = "?",    .toktype = TOK_QSTN   },
	{ .intext = ":",    .toktype = TOK_COLON  },
	{ .intext = NULL,   .toktype = TOK_NONE   }
};

tok_t *tok_read(FILE *fd)
{
	//Start token list with a single empty token for beginning the file
	tok_t *start_tok = alloc_mandatory(sizeof(tok_t));
	start_tok->type = TOK_FILE;
	start_tok->text = alloc_mandatory(1);
	start_tok->text[0] = '\0';
	
	tok_t *tail = start_tok;
	
	//Load the file into memory and handle splicing/trigraphs
	char *buf = charin_read(fd);
	
	//In tokenizing, we'll need to use a special-case to enable recognition of < > string constants.
	bool include_directive = false;
	
	//Track whether we skipped any whitespace before each token
	bool immediate = true;
	
	//Work through the whole file, building tokens from it
	char *next = buf;
	while(1)
	{		
		//Skip over stuff that's not a token.
		//Keep skipping over any possible not-a-token until none of them match.
		char *next_old = next;
	
		//Skip whitespace (except newlines)
		while(isspace(next[0]) && next[0] != '\n')
		{
			next++;
			immediate = false;
		}
		
		if(next_old != next)
			continue;
		
		//If a line comment is encountered, skip the rest of the line until the newline
		if(next[0] == '/' && next[1] == '/')
		{
			while(next[0] != '\n' && next[0] != '\0')
			{
				next++;
				immediate = false;
			}
		}
		
		if(next_old != next)
			continue;
		
		//If a block comment is encountered, skip until it ends
		if(next[0] == '/' && next[1] == '*')
		{
			//Scan for terminating "*" "/" pair
			while(!(next[0] == '*' && next[1] == '/'))
			{
				if(next[0] == '\0' || next[1] == '\0')
				{
					fprintf(stderr, "end-of-file encountered during block comment\n");
					exit(-1);
				}
				
				next++;
				immediate = false;
			}
			
			//Skip the terminating pair
			next += 2;
		}
		
		if(next_old != next)
			continue;

		//Alright, we're at the beginning of the next token.
		//Allocate storage for next token and link into end of list
		tok_t *tok = alloc_mandatory(sizeof(tok_t));
		tok->prev = tail;
		tail->next = tok;
		tail = tok;
		
		//Store whether-or-not we skipped any whitespace before the token, and reset that tracking
		tok->immediate = immediate;
		immediate = true;
		
		//Look ahead to find the type and length of token we're looking at.
		size_t len = 0;
		
		if(next[0] == '\n') //Check for newline as a token
		{
			//Newline
			tok->type = TOK_NEWLINE;
			len = 1;
			
			//End special-case handling of < > strings if enabled
			include_directive = false;
		}
		else if(next[0] == '\'') //Check for character constants
		{
			//This is a character constant and runs until another non-escaped single-quote.
			tok->type = TOK_CHARACTER;
			len = 1;
			while(1)
			{
				if(next[len] == '\\' && next[len+1] != '\0')
				{
					//Escaped character during a character constant, character contains both characters and continues
					len += 2;
					continue;
				}
				
				if(next[len] == '\'')
				{
					//Unescaped single-quote terminates the character constant
					len++;
					break;
				}
				
				if(next[len] == '\0')
				{
					//End-of-file during a character constant is an error
					fprintf(stderr, "end-of-file encountered during character constant\n");
					exit(-1);
				}
				
				//Other characters are consumed
				len++;
				continue;
			}
		}
		else if(next[0] == '"') //Check for string constants
		{
			//This is a string constant, and runs until another non-escaped double-quote.
			tok->type = TOK_STRLIT;
			len = 1;
			while(1)
			{
				if(next[len] == '\\' && next[len+1] != '\0')
				{
					//Escaped character during a string, string contains both characters and continues
					len += 2;
					continue;
				}
				
				if(next[len] == '"')
				{
					//Unescaped double-quote terminates the string
					len++;
					break;
				}
				
				if(next[len] == '\0')
				{
					//End-of-file during a string is an error
					fprintf(stderr, "end-of-file encountered during string\n");
					exit(-1);
				}
				
				//Other characters are consumed
				len++;
				continue;
			}
		}
		else if(next[0] == '<' && include_directive) //Check for < > strings but only in special cases
		{
			//This is a header filename in an include directive.
			//It works like a string constant but using < > instead of doublequotes.
			tok->type = TOK_SYSHDR;
			len = 1;
			while(1)
			{
				if(next[len] == '>')
				{
					//Greater-than terminates the string
					len++;
					break;
				}
				
				if(next[len] == '\0')
				{
					//End-of-file during a string is an error
					fprintf(stderr, "end-of-file encountered during string\n");
					exit(-1);
				}
				
				//Other characters are consumed
				len++;
				continue;
			}
		}
		else if(isdigit(next[0]) || (next[0] == '.' && isdigit(next[1]))) //Check for preprocessor numbers
		{
			//Digit or period-and-digit. This is a preprocessor number.
			//It contains a sequence of digits, periods, letters, underscores, and exponent notations.
			//Find out how long it is.
			tok->type = TOK_PNUMBER;
			while(1)
			{
				//Preprocessor numbers include the exponent notations: e+ E+ p+ P+ e- E- p- P-
				if(tolower(next[len]) == 'e' || tolower(next[len]) == 'p')
				{
					if(next[len+1] == '+' || next[len+1] == '-')
					{
						len += 2;
						continue;
					}
				}
				
				//Preprocessor numbers include periods and underscores.
				if(next[len] == '.' || next[len] == '_')
				{
					len++;
					continue;
				}
				
				//Preprocessor numbers include alphanumeric characters.
				if(isalnum(next[len]))
				{
					len++;
					continue;
				}
				
				//Any other character terminates a preprocessor number.
				break;
			}
		}
		else if(isalpha(next[0]) || (next[0] == '_')) //Check for identifiers
		{
			//Letter or underscore. This is an identifier.
			//It consists of a sequence of letters, underscores, and digits.
			//Find out how long it is.
			tok->type = TOK_IDENT;
			while(1)
			{
				//Identifiers include letters, numbers, and underscores.
				if(isalnum(next[len]) || (next[len] == '_'))
				{
					len++;
					continue;
				}
				
				//Any other character terminates an identifier.
				break;
			}
		}
		else if(next[0] == '\0') //Check for end-of-file
		{
			//Reached end of buffer containing the file. Emit EOF token.
			tok->type = TOK_EOF;
			len = 0;
		}
		else //Everything else is punctuation
		{
			//Check for each possible punctuator
			const tok_punct_t *punct = NULL;
			for(int pp = 0; tok_punct_array[pp].intext != NULL; pp++)
			{
				//See if the next few characters of input match the entire punctuator definition
				const tok_punct_t *check = &(tok_punct_array[pp]);
				if(!strncmp(next, check->intext, strlen(check->intext)))
				{
					punct = check;
					break;
				}
			}
			
			if(punct != NULL)
			{
				//Matched punctuation.
				tok->type = punct->toktype;
				len = strlen(punct->intext);
			}
			else
			{
				
				//Any other text is single-character junk
				tok->type = TOK_JUNK;
				len = 1;
			}
		}
		
		//Allocate a buffer and store text of the token
		tok->text = alloc_mandatory(len+1);
		memcpy(tok->text, next, len);
		tok->text[len] = '\0';
	
		//Special case - enable handling of < > strings if we just hit an include directive.
		if(tok->prev->type == TOK_HASH && !strcmp(tok->text, "include"))
		{
			include_directive = true;
		}
		
		//Stop parsing if we hit end-of-file; otherwise, move past the token and continue
		if(next[0] == '\0')
		{
			break;
		}
		else
		{
			next += len;
		}
	}
	
	//Get rid of original text buffer and return the file as a token list.
	free(buf);
	return start_tok;
}

const char *tok_typename(const tok_t *tok)
{
	assert(tok->type >= TOK_NONE && tok->type < TOK_MAX);
	return tok_typenames[tok->type];
}

void tok_delete_single(tok_t *tok)
{
	assert(tok != NULL && tok->type != TOK_NONE);
	
	if(tok->prev != NULL)
		tok->prev->next = tok->next;
	
	if(tok->next != NULL)
		tok->next->prev = tok->prev;
	
	if(tok->text != NULL)
		free(tok->text);
	
	if(tok->macros != NULL)
	{
		for(size_t mm = 0; mm < tok->nmacros; mm++)
		{
			free(tok->macros[mm]);
		}
		free(tok->macros);
	}
	
	tok->prev = NULL;
	tok->next = NULL;
	tok->text = NULL;
	tok->type = TOK_NONE;
	free(tok);
}

void tok_delete_range(tok_t *first, tok_t *last)
{
	while(1)
	{
		//Sanity-check - if parameters are bad, we could run off the end of a token list entirely.
		if(first == NULL)
		{
			fprintf(stderr, "bad token range\n");
			abort();
		}
		
		//If there's only one token to delete, delete it and we're done
		if(first == last)
		{
			tok_delete_single(first);
			return;
		}
		
		//Otherwise, set aside the token immediately following the first, and then delete the first.
		tok_t *fnext = first->next;
		tok_delete_single(first);
		
		//Continue deleting from that token
		first = fnext;
	}
}

void tok_delete_all(tok_t *first)
{
	while(first != NULL)
	{
		tok_t *fnext = first->next;
		tok_delete_single(first);
		first = fnext;
	}
}

tok_t *tok_copy(tok_t *first, tok_t *last)
{
	//Handle trivial case
	if(first == NULL)
		return NULL;
	
	//Make the first token entry we'll return
	tok_t *retval = alloc_mandatory(sizeof(tok_t));
	tok_t *tail = retval;

	while(1)
	{
		//Sanity-check that we didn't run off the list
		if(first == NULL)
		{
			fprintf(stderr, "bad token copy\n");
			abort();
		}
		
		assert(first->type != TOK_NONE);
		
		//Copy the data from the next-up source token
		tail->type = first->type;
		tail->text = strdup_mandatory(first->text);
		tail->immediate = first->immediate;
		tail->line = first->line;
		
		tail->nmacros = first->nmacros;
		if(tail->nmacros > 0)
		{
			tail->macros = alloc_mandatory(tail->nmacros * sizeof(char*));
			for(size_t mm = 0; mm < tail->nmacros; mm++)
			{
				tail->macros[mm] = strdup_mandatory(first->macros[mm]);
			}
		}
		
		//If we just copied the last token, we're done
		if(first == last)
		{
			return retval;
		}
		
		//Otherwise, allocate and link another token, and advance in the source list
		tok_t *ntail = alloc_mandatory(sizeof(tok_t));		
		ntail->prev = tail;
		tail->next = ntail;
		
		first = first->next;
		tail = ntail;
	}
}

tok_t *tok_copy_all(tok_t *first)
{
	if(first == NULL)
		return NULL;
	
	//Make the first token entry we'll return
	tok_t *retval = alloc_mandatory(sizeof(tok_t));
	tok_t *tail = retval;

	while(1)
	{
		assert(first->type != TOK_NONE);
		
		//Copy the data from the next-up source token
		tail->type = first->type;
		tail->text = strdup_mandatory(first->text);
		tail->immediate = first->immediate;
		tail->line = first->line;
		
		tail->nmacros = first->nmacros;
		if(tail->nmacros > 0)
		{
			tail->macros = alloc_mandatory(tail->nmacros * sizeof(char*));
			for(size_t mm = 0; mm < tail->nmacros; mm++)
			{
				tail->macros[mm] = strdup_mandatory(first->macros[mm]);
			}
		}
		
		//If we just copied the last token, we're done
		if(first->next == NULL)
		{
			return retval;
		}
		
		//Otherwise, allocate and link another token, and advance in the source list
		tok_t *ntail = alloc_mandatory(sizeof(tok_t));		
		ntail->prev = tail;
		tail->next = ntail;
		
		first = first->next;
		tail = ntail;
	}	
}

void tok_err(tok_t *violator, const char *str, ...)
{
	va_list ap;
	va_start(ap, str);
	
	fprintf(stderr, "[%s]:", violator->text);
	vfprintf(stderr, str, ap);
	fprintf(stderr, "\n");
	
	va_end(ap);
	
	exit(-1);
}


void tok_pass_keyw(tok_t *list)
{
	for(tok_t *tt = list; tt != NULL; tt = tt->next)
	{
		if(tt->type == TOK_IDENT)
		{
			//Check if the preprocessor-identifier is actually a keyword.
			for(tok_type_t kk = TOK_KEYW_S + 1; kk < TOK_KEYW_E; kk++)
			{
				if(!strcmp(tt->text, tok_typenames[kk]))
				{
					tt->type = kk;
					break;
				}
			}
		}
	}
}

void tok_pass_nowh(tok_t *list)
{
	tok_t *tt = list;
	while(tt != NULL)
	{
		tok_t *ttn = tt->next;
		
		if(tt->type == TOK_EOF || tt->type == TOK_NEWLINE)
		{
			tok_delete_single(tt);
		}
		
		tt = ttn;
	}
}

void tok_pass_nums(tok_t *list)
{
	for(tok_t *tt = list; tt != NULL; tt = tt->next)
	{
		if(tt->type == TOK_PNUMBER)
		{
			//For now just split numbers into int/float without trying to parse their value.
			//We'll have a "determine consts" pass later that actually assigns values to syntaxes.
			if(strchr(tt->text, '+') || strchr(tt->text, '-') || strchr(tt->text, '.'))
			{
				//Token contains an exponent or period. Consider it a float.
				tt->type = TOK_FLTC;
			}
			else
			{
				//No exponent/period. Consider it an int.
				tt->type = TOK_INTC;
			}
		}
	}	
}
