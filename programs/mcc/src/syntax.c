//syntax.c
//Syntax definition and matching
//Bryan E. Topp <betopp@betopp.com> 2021

#include "syntax.h"
#include "consts.h"
#include "alloc.h"
#include "tinfo.h"
#include "macro.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>

//Definition of C99 syntax. Possible sequences that make up each syntax element.
//Major dimension is the syntax element being constructed - the larger one, which may contain others.
//Middle dimension is each possibility for constructing that syntax element.
//Minor dimension is each element necessary to satisfy that possibility.
//In the minor dimension, the first element will be stored to indicate "which possibility".
//The last element should be 0 to terminate the list.
static const int syntax_options[S_MAX][SYNTAX_OPTIONS][SYNTAX_FANOUT+2] = 
{
	[S_PRIMARY_EXPRESSION] = 
	{
		{'i', TOK_IDENT, 0},
		{'c', S_CONSTANT, 0},
		{'s', TOK_STRLIT, 0},
		{'e', TOK_PARENL, S_EXPRESSION, TOK_PARENR, 0},
		{0}
	},
	[S_POSTFIX_EXPRESSION] = 
	{
		{'e', S_PRIMARY_EXPRESSION, 0},
		{'s', S_POSTFIX_EXPRESSION, TOK_BRACKL, S_EXPRESSION, TOK_BRACKR, 0},
		{'c', S_POSTFIX_EXPRESSION, TOK_PARENL, S_ARGUMENT_EXPRESSION_LIST, TOK_PARENR, 0},
		{'C', S_POSTFIX_EXPRESSION, TOK_PARENL, TOK_PARENR, 0},
		{'d', S_POSTFIX_EXPRESSION, TOK_DOT, TOK_IDENT, 0},
		{'a', S_POSTFIX_EXPRESSION, TOK_ARROW, TOK_IDENT, 0},
		{'p', S_POSTFIX_EXPRESSION, TOK_DPLUS, 0},
		{'m', S_POSTFIX_EXPRESSION, TOK_DMINUS, 0},
		{'i', TOK_PARENL, S_TYPE_NAME, TOK_PARENR, TOK_BRACEL, S_INITIALIZER_LIST, TOK_COMMA, TOK_BRACER, 0},
		{'I', TOK_PARENL, S_TYPE_NAME, TOK_PARENR, TOK_BRACEL, S_INITIALIZER_LIST, TOK_BRACER, 0},
		{0}
	},
	[S_ARGUMENT_EXPRESSION_LIST] = 
	{
		{'a', S_ASSIGNMENT_EXPRESSION, 0},
		{'l', S_ARGUMENT_EXPRESSION_LIST, TOK_COMMA, S_ASSIGNMENT_EXPRESSION, 0},
		{0}
	},
	[S_ASSIGNMENT_EXPRESSION] = 
	{
		{'c', S_CONDITIONAL_EXPRESSION, 0},
		{'u', S_UNARY_EXPRESSION, S_ASSIGNMENT_OPERATOR, S_ASSIGNMENT_EXPRESSION, 0},
		{0}
	},
	[S_CONSTANT_EXPRESSION] = 
	{
		{'c', S_CONDITIONAL_EXPRESSION, 0},
		{0}
	},
	[S_CONDITIONAL_EXPRESSION] = 
	{
		{'s', S_LOGICAL_OR_EXPRESSION, 0},
		{'t', S_LOGICAL_OR_EXPRESSION, TOK_QSTN, S_EXPRESSION, TOK_COLON, S_CONDITIONAL_EXPRESSION, 0},
		{0}
	},
	[S_LOGICAL_OR_EXPRESSION] = 
	{
		{'a', S_LOGICAL_AND_EXPRESSION, 0},
		{'o', S_LOGICAL_OR_EXPRESSION, TOK_DBAR, S_LOGICAL_AND_EXPRESSION, 0},
		{0}
	},
	[S_LOGICAL_AND_EXPRESSION] = 
	{
		{'o', S_INCLUSIVE_OR_EXPRESSION, 0},
		{'a', S_LOGICAL_AND_EXPRESSION, TOK_DAMP, S_INCLUSIVE_OR_EXPRESSION, 0},
		{0}
	},
	[S_INCLUSIVE_OR_EXPRESSION] = 
	{
		{'e', S_EXCLUSIVE_OR_EXPRESSION, 0},
		{'i', S_INCLUSIVE_OR_EXPRESSION, TOK_BAR, S_EXCLUSIVE_OR_EXPRESSION, 0},
		{0}
	},
	[S_EXCLUSIVE_OR_EXPRESSION] = 
	{
		{'a', S_AND_EXPRESSION, 0},
		{'x', S_EXCLUSIVE_OR_EXPRESSION, TOK_CARAT, S_AND_EXPRESSION, 0},
		{0}
	},
	[S_AND_EXPRESSION] = 
	{
		{'e', S_EQUALITY_EXPRESSION, 0},
		{'a', S_AND_EXPRESSION, TOK_AMP, S_EQUALITY_EXPRESSION, 0},
		{0}
	},
	[S_EQUALITY_EXPRESSION] = 
	{
		{'r', S_RELATIONAL_EXPRESSION, 0},
		{'e', S_EQUALITY_EXPRESSION, TOK_DEQ, S_RELATIONAL_EXPRESSION, 0},
		{'n', S_EQUALITY_EXPRESSION, TOK_EXCEQ, S_RELATIONAL_EXPRESSION, 0},
		{0}
	},
	[S_RELATIONAL_EXPRESSION] = 
	{
		{'s', S_SHIFT_EXPRESSION, 0},
		{'l', S_RELATIONAL_EXPRESSION, TOK_LT, S_SHIFT_EXPRESSION, 0},
		{'g', S_RELATIONAL_EXPRESSION, TOK_GT, S_SHIFT_EXPRESSION, 0},
		{'L', S_RELATIONAL_EXPRESSION, TOK_LEQ, S_SHIFT_EXPRESSION, 0},
		{'G', S_RELATIONAL_EXPRESSION, TOK_GEQ, S_SHIFT_EXPRESSION, 0},
		{0}
	},
	[S_SHIFT_EXPRESSION] = 
	{
		{'a', S_ADDITIVE_EXPRESSION, 0},
		{'l', S_SHIFT_EXPRESSION, TOK_DLT, S_ADDITIVE_EXPRESSION, 0},
		{'r', S_SHIFT_EXPRESSION, TOK_DGT, S_ADDITIVE_EXPRESSION, 0},
		{0}
	},
	[S_ADDITIVE_EXPRESSION] = 
	{
		{'t', S_MULTIPLICATIVE_EXPRESSION, 0},
		{'p', S_ADDITIVE_EXPRESSION, TOK_PLUS, S_MULTIPLICATIVE_EXPRESSION, 0},
		{'m', S_ADDITIVE_EXPRESSION, TOK_MINUS, S_MULTIPLICATIVE_EXPRESSION, 0},
		{0}
	},
	[S_MULTIPLICATIVE_EXPRESSION] = 
	{
		{'c', S_CAST_EXPRESSION, 0},
		{'t', S_MULTIPLICATIVE_EXPRESSION, TOK_ASTER, S_CAST_EXPRESSION, 0},
		{'d', S_MULTIPLICATIVE_EXPRESSION, TOK_SLASH, S_CAST_EXPRESSION, 0},
		{'m', S_MULTIPLICATIVE_EXPRESSION, TOK_PCT, S_CAST_EXPRESSION, 0},
		{0}
	},
	[S_CAST_EXPRESSION] = 
	{
		{'u', S_UNARY_EXPRESSION, 0},
		{'c', TOK_PARENL, S_TYPE_NAME, TOK_PARENR, S_CAST_EXPRESSION, 0},
		{0}
	},
	[S_UNARY_EXPRESSION] = 
	{
		{'p', S_POSTFIX_EXPRESSION, 0},
		{'i', TOK_DPLUS, S_UNARY_EXPRESSION, 0},
		{'d', TOK_DMINUS, S_UNARY_EXPRESSION, 0},
		{'u', S_UNARY_OPERATOR, S_CAST_EXPRESSION, 0},
		{'s', TOK_SIZEOF, S_UNARY_EXPRESSION, 0},
		{'S', TOK_SIZEOF, TOK_PARENL, S_TYPE_NAME, TOK_PARENR, 0},
		{0}
	},
	[S_UNARY_OPERATOR] = 
	{
		{'a', TOK_AMP, 0},
		{'t', TOK_ASTER, 0},
		{'p', TOK_PLUS, 0},
		{'m', TOK_MINUS, 0},
		{'n', TOK_TILDE, 0},
		{'e', TOK_EXCL, 0},
		{0}
	},
	[S_CONSTANT] = 
	{
		{'i', TOK_INTC, 0},
		{'f', TOK_FLTC, 0},
		{'e', S_ENUMERATION_CONSTANT, 0},
		{'c', TOK_CHARACTER, 0},
		{0}
	},
	[S_ENUMERATION_CONSTANT] =
	{
		{'i', TOK_IDENT, 0},
		{0}
	},
	[S_ASSIGNMENT_OPERATOR] = 
	{
		{'e', TOK_EQU, 0},
		{'t', TOK_ASTEQ, 0},
		{'s', TOK_SLSHEQ, 0},
		{'r', TOK_PCTEQ, 0},
		{'p', TOK_PLUSEQ, 0},
		{'m', TOK_MINEQ, 0},
		{'L', TOK_DLEQ,  0},
		{'R', TOK_DGEQ,  0},
		{'a', TOK_AMPEQ, 0},
		{'x', TOK_CAREQ, 0},
		{'b', TOK_BAREQ, 0},
		{0}		
	},
	[S_EXPRESSION] =
	{
		{'a', S_ASSIGNMENT_EXPRESSION, 0},
		{'e', S_EXPRESSION, TOK_COMMA, S_ASSIGNMENT_EXPRESSION, 0},
		{0}
	},
	[S_TYPE_NAME] = 
	{
		{'q', S_SPECIFIER_QUALIFIER_LIST, S_ABSTRACT_DECLARATOR, 0},
		{'Q', S_SPECIFIER_QUALIFIER_LIST, 0},
	},
	[S_SPECIFIER_QUALIFIER_LIST] = 
	{
		{'s', S_TYPE_SPECIFIER, 0},
		{'S', S_TYPE_SPECIFIER, S_SPECIFIER_QUALIFIER_LIST, 0},
		{'q', S_TYPE_QUALIFIER, 0},
		{'Q', S_TYPE_QUALIFIER, S_SPECIFIER_QUALIFIER_LIST, 0},
		{0},
	},
	[S_TYPE_SPECIFIER] = 
	{
		{'v', TOK_VOID, 0},
		{'c', TOK_CHAR, 0},
		{'s', TOK_SHORT, 0},
		{'i', TOK_INT, 0},
		{'l', TOK_LONG, 0},
		{'f', TOK_FLOAT, 0},
		{'d', TOK_DOUBLE, 0},
		{'g', TOK_SIGNED, 0},
		{'u', TOK_UNSIGNED, 0},
		{'b', TOK_BOOL, 0},
		{'p', TOK_COMPLEX, 0},
		{'S', S_STRUCT_OR_UNION_SPECIFIER, 0},
		{'E', S_ENUM_SPECIFIER, 0},
		{'T', S_TYPEDEF_NAME, 0},
		{0}
	},
	[S_ABSTRACT_DECLARATOR] = 
	{
		{'p', S_POINTER, 0},
		{'d', S_POINTER, S_DIRECT_ABSTRACT_DECLARATOR, 0},
		{'D', S_DIRECT_ABSTRACT_DECLARATOR, 0},
		{0}
	},
	[S_DIRECT_ABSTRACT_DECLARATOR] = 
	{
		{'n', TOK_PARENL, S_ABSTRACT_DECLARATOR, TOK_PARENR, 0},
		{'b', S_DIRECT_ABSTRACT_DECLARATOR, TOK_BRACKL, S_ASSIGNMENT_EXPRESSION, TOK_BRACKR, 0},
		{'B', TOK_BRACKL, S_ASSIGNMENT_EXPRESSION, TOK_BRACKR, 0},
		{'c', S_DIRECT_ABSTRACT_DECLARATOR, TOK_BRACKL, TOK_BRACKR, 0},
		{'C', TOK_BRACKL, TOK_BRACKR, 0},
		{'s', S_DIRECT_ABSTRACT_DECLARATOR, TOK_BRACKL, TOK_ASTER, TOK_BRACKR, 0},
		{'S', TOK_BRACKL, TOK_ASTER, TOK_BRACKR, 0},
		{'p', S_DIRECT_ABSTRACT_DECLARATOR, TOK_PARENL, S_PARAMETER_TYPE_LIST, TOK_PARENR, 0},
		{'P', S_DIRECT_ABSTRACT_DECLARATOR, TOK_PARENL, TOK_PARENR, 0},
		{'q', TOK_PARENL, S_PARAMETER_TYPE_LIST, TOK_PARENR, 0},
		{'Q', TOK_PARENL, TOK_PARENR, 0},
		{0}
	},
	[S_PARAMETER_TYPE_LIST] = 
	{
		{'l', S_PARAMETER_LIST, 0},
		{'L', S_PARAMETER_LIST, TOK_COMMA, TOK_ELLIPS, 0},
		{0}
	},
	[S_PARAMETER_LIST] = 
	{
		{'d', S_PARAMETER_DECLARATION, 0},
		{'D', S_PARAMETER_LIST, TOK_COMMA, S_PARAMETER_DECLARATION, 0},
		{0}
	},
	[S_TYPE_QUALIFIER] = 
	{
		{'c', TOK_CONST, 0},
		{'r', TOK_RESTRICT, 0},
		{'v', TOK_VOLATILE, 0},
		{0}
	},
	[S_STRUCT_OR_UNION_SPECIFIER] = 
	{
		{'b', S_STRUCT_OR_UNION, TOK_IDENT, TOK_BRACEL, S_STRUCT_DECLARATION_LIST, TOK_BRACER, 0},
		{'B', S_STRUCT_OR_UNION, TOK_BRACEL, S_STRUCT_DECLARATION_LIST, TOK_BRACER, 0},
		{'i', S_STRUCT_OR_UNION, TOK_IDENT, 0},
		{0}
	},
	[S_ENUM_SPECIFIER] = 
	{
		{'l', TOK_ENUM, TOK_IDENT, TOK_BRACEL, S_ENUMERATOR_LIST, TOK_BRACER, 0},
		{'L', TOK_ENUM, TOK_BRACEL, S_ENUMERATOR_LIST, TOK_BRACER, 0},
		{'c', TOK_ENUM, TOK_IDENT, TOK_BRACEL, S_ENUMERATOR_LIST, TOK_COMMA, TOK_BRACER, 0},
		{'C', TOK_ENUM, TOK_BRACEL, S_ENUMERATOR_LIST, TOK_COMMA, TOK_BRACER, 0},
		{'e', TOK_ENUM, TOK_IDENT, 0},
		{0}
	},
	[S_TYPEDEF_NAME] = 
	{
		{'i', TOK_IDENT, 0},
		{0}
	},
	[S_POINTER] = 
	{
		{'n', TOK_ASTER, S_TYPE_QUALIFIER_LIST, 0},
		{'N', TOK_ASTER, 0},
		{'p', TOK_ASTER, S_TYPE_QUALIFIER_LIST, S_POINTER, 0},
		{'P', TOK_ASTER, S_POINTER, 0},
		{0}
	},
	[S_PARAMETER_DECLARATION] = 
	{
		{'d', S_DECLARATION_SPECIFIERS, S_DECLARATOR, 0},
		{'a', S_DECLARATION_SPECIFIERS, S_ABSTRACT_DECLARATOR, 0},
		{'A', S_DECLARATION_SPECIFIERS, 0},
		{0}
	},
	[S_STRUCT_OR_UNION] = 
	{
		{'s', TOK_STRUCT, 0},
		{'u', TOK_UNION, 0},
		{0}
	},
	[S_STRUCT_DECLARATION_LIST] = 
	{
		{'d', S_STRUCT_DECLARATION, 0},
		{'l', S_STRUCT_DECLARATION_LIST, S_STRUCT_DECLARATION, 0},
		{0}
	},
	[S_ENUMERATOR_LIST] = 
	{
		{'e', S_ENUMERATOR, 0},
		{'l', S_ENUMERATOR_LIST, TOK_COMMA, S_ENUMERATOR, 0},
		{0},
	},
	[S_TYPE_QUALIFIER_LIST] = 
	{
		{'t', S_TYPE_QUALIFIER, 0},
		{'l', S_TYPE_QUALIFIER_LIST, S_TYPE_QUALIFIER, 0},
		{0},
	},
	[S_DECLARATION_SPECIFIERS] = 
	{
		{'c', S_STORAGE_CLASS_SPECIFIER, S_DECLARATION_SPECIFIERS, 0},
		{'C', S_STORAGE_CLASS_SPECIFIER, 0},
		{'t', S_TYPE_SPECIFIER, S_DECLARATION_SPECIFIERS, 0},
		{'T', S_TYPE_SPECIFIER, 0},
		{'q', S_TYPE_QUALIFIER, S_DECLARATION_SPECIFIERS, 0},
		{'Q', S_TYPE_QUALIFIER, 0},
		{'f', S_FUNCTION_SPECIFIER, S_DECLARATION_SPECIFIERS, 0},
		{'F', S_FUNCTION_SPECIFIER, 0},
		{0}
	},
	[S_DECLARATOR] = 
	{
		{'p', S_POINTER, S_DIRECT_DECLARATOR, 0},
		{'P', S_DIRECT_DECLARATOR, 0},
		{0}
	},
	[S_STRUCT_DECLARATION] =
	{
		{'q', S_SPECIFIER_QUALIFIER_LIST, S_STRUCT_DECLARATOR_LIST, 0},
		{0}
	},
	[S_ENUMERATOR] = 
	{
		{'c', S_ENUMERATION_CONSTANT, 0},
		{'e', S_ENUMERATION_CONSTANT, TOK_EQU, S_CONSTANT_EXPRESSION, 0},
		{0}
	},
	[S_STORAGE_CLASS_SPECIFIER] =
	{
		{'t', TOK_TYPEDEF, 0},
		{'e', TOK_EXTERN, 0},
		{'s', TOK_STATIC, 0},
		{'a', TOK_AUTO, 0},
		{'r', TOK_REGISTER, 0},
		{0}
	},
	[S_FUNCTION_SPECIFIER] = 
	{
		{'i', TOK_INLINE, 0},
		{0}
	},
	[S_DIRECT_DECLARATOR] = 
	{
		{'i', TOK_IDENT, 0},
		{'p', TOK_PARENL, S_DECLARATOR, TOK_PARENR, 0},
		{'q', S_DIRECT_DECLARATOR, TOK_BRACKL, S_TYPE_QUALIFIER_LIST, S_ASSIGNMENT_EXPRESSION, TOK_BRACKR, 0},
		{'Q', S_DIRECT_DECLARATOR, TOK_BRACKL, S_TYPE_QUALIFIER_LIST, TOK_BRACKR, 0},
		{'r', S_DIRECT_DECLARATOR, TOK_BRACKL, S_ASSIGNMENT_EXPRESSION, TOK_BRACKR, 0},
		{'R', S_DIRECT_DECLARATOR, TOK_BRACKL, TOK_BRACKR, 0},
		{'s', S_DIRECT_DECLARATOR, TOK_BRACKL, TOK_STATIC, S_TYPE_QUALIFIER_LIST, S_ASSIGNMENT_EXPRESSION, TOK_BRACKR, 0},
		{'S', S_DIRECT_DECLARATOR, TOK_BRACKL, TOK_STATIC, S_ASSIGNMENT_EXPRESSION, TOK_BRACKR, 0},
		{'t', S_DIRECT_DECLARATOR, TOK_BRACKL, S_TYPE_QUALIFIER_LIST, TOK_STATIC, S_ASSIGNMENT_EXPRESSION, TOK_BRACKR, 0},
		{'a', S_DIRECT_DECLARATOR, TOK_BRACKL, S_TYPE_QUALIFIER_LIST, TOK_ASTER, TOK_BRACKR, 0},
		{'A', S_DIRECT_DECLARATOR, TOK_BRACKL, TOK_ASTER, TOK_BRACKR, 0},
		{'l', S_DIRECT_DECLARATOR, TOK_PARENL, S_PARAMETER_TYPE_LIST, TOK_PARENR, 0},
		{'i', S_DIRECT_DECLARATOR, TOK_PARENL, S_IDENTIFIER_LIST, TOK_PARENR, 0},
		{'I', S_DIRECT_DECLARATOR, TOK_PARENL, TOK_PARENR, 0},
		{0}
	},
	[S_STRUCT_DECLARATOR_LIST] = 
	{
		{'d', S_STRUCT_DECLARATOR, 0},
		{'l', S_STRUCT_DECLARATOR_LIST, TOK_COMMA, S_STRUCT_DECLARATOR, 0},
		{0}
	},
	[S_IDENTIFIER_LIST] = 
	{
		{'i', TOK_IDENT, 0},
		{'l', S_IDENTIFIER_LIST, TOK_COMMA, TOK_IDENT, 0},
		{0}
	},
	[S_STRUCT_DECLARATOR] = 
	{
		{'d', S_DECLARATOR, 0},
		{'c', S_DECLARATOR, TOK_COLON, S_CONSTANT_EXPRESSION, 0},
		{'C', TOK_COLON, S_CONSTANT_EXPRESSION, 0},
		{0}
	},
};

//Frees a syntax node and all children.
void syntax_free(syntax_node_t *n)
{
	for(int cc = 0; cc < SYNTAX_FANOUT; cc++)
	{
		if(n->children[cc] == NULL)
			break;
		
		syntax_free(n->children[cc]);
	}
	free(n);
}

//Tries to match a single possible construction of the given syntax
static syntax_node_t *syntax_try_option(syntax_type_t type, int option, tok_t *start, tok_t *end)
{
	//See if this option begins with a left-recursion.
	//If so, we need to prevent infinite recursion in trying to match children.
	//The left-recursive option will contain some token following its left-recursion.
	//Necessarily, we'll consume one such token if we match the option at this level.
	//So pull back the end prior to that token when recursing deeper.
	tok_t *lrend = end;
	if(syntax_options[type][option][1] == type)
	{
		//Left-recursive definition.
		//Find a token that gets consumed by the definition to limit our recursion.
		tok_type_t limiting_tok = 0;
		for(int tt = 1; syntax_options[type][option][tt] != 0; tt++)
		{
			int required = syntax_options[type][option][tt];
			if(required > TOK_NONE && required < TOK_MAX)
			{
				limiting_tok = required;
				break;
			}
		}
		
		//Left-recursive definitions all contain at least one token directly
		assert(limiting_tok != 0);
		
		//Back up the ending token until we consume one of these tokens
		while( (lrend != start) && (lrend->type != limiting_tok) )
		{
			lrend = lrend->prev;
		}
		
		if(lrend == start)
		{
			//Didn't find the token we'll need in the left-recursive definition.
			//We can't use this left-recursive option.
			//(Or, the token we found is literally the first token, which doesn't work either.)
			return NULL;
		}
		
		//Found the token we need at this depth - limit recursive search to one token prior.
		lrend = lrend->prev;
	}
	
	//Allocate space for the node
	syntax_node_t *n = alloc_mandatory(sizeof(syntax_node_t));
	n->type = type;
	n->option = syntax_options[type][option][0];
	n->start = start;
	n->ntoks = 0;
	
	//Remaining tokens that further children should match
	tok_t *unmatched = start;
	
	//Try to match all children
	for(int chidx = 0; chidx < SYNTAX_FANOUT; chidx++)
	{
		//See if there's no more children to match - if so, we've succeeded
		syntax_type_t chtype = syntax_options[type][option][chidx+1];
		if(chtype == 0)
			break;
		
		//See if we can use the whole remaining token series (normal children)
		//or if we limit how much we consume (left-recursive children)
		tok_t *chend = end;
		if(chidx == 0 && lrend != NULL)
			chend = lrend; //Left-recursive - limit range so we don't infinitely recurse
		
		//Attempt to match the child with remaining input tokens
		n->children[chidx] = syntax_try(chtype, unmatched, chend);
		if(n->children[chidx] == NULL)
		{
			//Failed to match this child.
			//Free any children that we did match.
			while(chidx > 0)
			{
				chidx--;
				syntax_free(n->children[chidx]);
			}
			
			//Couldn't match all children.
			free(n);
			return NULL;
		}
		
		//Matched this child. Advance past the tokens it used.
		n->end = n->children[chidx]->end;
		unmatched = n->end->next;
		n->ntoks += n->children[chidx]->ntoks;
	}
	
	//Success
	assert(n->ntoks > 0);
	return n;	
}


syntax_node_t *syntax_try(syntax_type_t type, tok_t *start, tok_t *end)
{
	//Validate type parameter
	if(type <= 0 || type >= S_MAX)
	{
		fprintf(stderr, "bad syntax table");
		abort();
	}
	
	//If we have no tokens left to match with, we cannot match any syntax.
	if(end->next == start)
	{
		return NULL;
	}
	
	//If we're trying to match a token, then this problem is trivial.
	//See if the input range begins with that token.
	if((int)type > TOK_NONE && (int)type < TOK_MAX)
	{
		if(start->type == (int)type)
		{
			//Token matches
			syntax_node_t *ts = alloc_mandatory(sizeof(syntax_node_t));
			ts->type = start->type;
			ts->start = start;
			ts->end = start;
			ts->ntoks = 1;
			
			return ts;
		}
		else
		{	
			//Token doesn't match
			return NULL;
		}
	}
	
	//All other syntax should have at least one way to construct it in our syntax table
	assert(syntax_options[type][0][0] != 0);
	
	//Pick the one that matches the longest sequence of tokens.
	syntax_node_t *longest_option = NULL;
	for(int option = 0; syntax_options[type][option][0] != 0; option++)
	{
		//Try to match the option
		syntax_node_t *option_result = syntax_try_option(type, option, start, end);
		if(option_result == NULL)
		{
			//Couldn't match this option
			continue;
		}
		else if(longest_option == NULL)
		{
			//First match
			longest_option = option_result;
		}
		else if(longest_option->ntoks < option_result->ntoks)
		{
			//Longer than existing match
			syntax_free(longest_option);
			longest_option = option_result;
		}
	}
		
	return longest_option;
}


void syntax_doconst(syntax_node_t *node)
{	
	assert(node->tinfo == NULL);
	assert(node->value == NULL);
	
	//Assume any nodes with only one child have the same type and value as that child.
	if(node->children[0] != NULL && node->children[1] == NULL)
	{
		syntax_doconst(node->children[0]);
		node->tinfo = node->children[0]->tinfo;
		node->value = node->children[0]->value;
		return;
	}
	
	//Handle integer constants
	if(node->type == (int)TOK_INTC)
	{
		consts_intc(node);
		return;
	}
	
	//Handle floating-point constants
	if(node->type == (int)TOK_FLTC)
	{
		consts_fltc(node);
		return;
	}
	
	//Handle logical operators
	if(node->type == S_LOGICAL_AND_EXPRESSION || node->type == S_LOGICAL_OR_EXPRESSION)
	{
		//Child 0 is left side, child 1 is operator, child 2 is right side.
		syntax_doconst(node->children[0]);
		syntax_doconst(node->children[2]);
		assert(node->children[1]->type == (int)TOK_DAMP || node->children[1]->type == (int)TOK_DBAR);
		
		//Result of logical operator always has int type; can be const if both children are const.
		node->tinfo = tinfo_for_basic(BTYPE_INT);
		if(node->children[0]->value != NULL && node->children[2]->value != NULL)
		{
			bool a_nz = tinfo_val_nz(node->children[0]->tinfo, node->children[0]->value);
			bool b_nz = tinfo_val_nz(node->children[2]->tinfo, node->children[2]->value);
			node->value = alloc_mandatory(sizeof(int));
			*(int*)(node->value) = a_nz && b_nz;
		}
		
		return;
	}
	
	//Handle postfix operators
	if(node->type == S_POSTFIX_EXPRESSION)
	{
		//Postfix expressions are a few different things.
		if(node->children[0]->type == (int)TOK_PARENL)
		{
			//Compound literal. Children 0 and 2 are parens.
			
			//Child 1 is the type of the literal.
			syntax_doconst(node->children[1]);
			node->tinfo = node->children[1]->tinfo;
			
			//Todo - parse compound literal for const value
			return;
		}
		if(node->children[1]->type == (int)TOK_PARENL)
		{
			//Function call. Child 0 is the function.
			syntax_doconst(node->children[0]);
			node->tinfo = node->children[0]->tinfo;
			
			//Child 2 is the argument list.
			if(node->children[2]->type != (int)TOK_PARENR)
				syntax_doconst(node->children[2]);
			
			//Function call is never const-able.
			return;
		}
	}
	
	//Handle unary operators
	if(node->type == S_UNARY_EXPRESSION)
	{
		if(node->children[0]->type == S_UNARY_OPERATOR)
		{
			//Unary operator followed by its operand.
			//Evaluate the operand.
			syntax_doconst(node->children[1]);
			
			//See what the operator is.
			if(node->children[0]->start->type == TOK_EXCL)
			{
				//Logical negation. Result is always int type. Result is constant if operand is.
				node->tinfo = tinfo_for_basic(BTYPE_INT);
				if(node->children[1]->value != NULL)
				{
					bool a_nz = tinfo_val_nz(node->children[1]->tinfo, node->children[1]->value);
					node->value = alloc_mandatory(sizeof(int));
					*(int*)(node->value) = a_nz ? 0 : 1;
				}
				return;
			}
			
			//if(node->children[0]->start->type == TOK_
		}
	}
	
	//Handle equality
	if(node->type == S_EQUALITY_EXPRESSION)
	{
		//Child 0 should be left side, child 2 should be right side
		syntax_doconst(node->children[0]);
		syntax_doconst(node->children[2]);
		assert(node->children[1]->type == (int)TOK_DEQ || node->children[1]->type == (int)TOK_EXCEQ);
		
		//Result of comparison always has type int, and is constant if both subexpressions are
		node->tinfo = tinfo_for_basic(BTYPE_INT);
		if(node->children[0]->value != NULL && node->children[2]->value != NULL)
		{
			node->value = alloc_mandatory(sizeof(int));
			bool invert = (node->children[1]->type == (int)TOK_EXCEQ);
			bool valeq = tinfo_val_eq(
				node->children[0]->tinfo, node->children[0]->value,
				node->children[2]->tinfo, node->children[2]->value);
			*(int*)(node->value) = (valeq ^ invert) ? 1 : 0;
		}
		
		return;
	}
		
	
	//Todo - handle other expressions
	abort();
	return;
}