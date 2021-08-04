//syntax.h
//Syntax definition and matching
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef SYNTAX_H
#define SYNTAX_H

#include "tok.h"

//How many different options can be available to construct each syntax element
#define SYNTAX_OPTIONS 16

//How many children each syntax node can have
#define SYNTAX_FANOUT 8

//All possible elements of C99 syntax.
typedef enum syntax_type_e
{
	S_START = TOK_MAX,
	
	S_PRIMARY_EXPRESSION,
	S_CONSTANT,
	S_EXPRESSION,
	S_POSTFIX_EXPRESSION,
	S_ARGUMENT_EXPRESSION_LIST,
	S_TYPE_NAME,
	S_INITIALIZER_LIST,
	S_ASSIGNMENT_EXPRESSION,
	S_CONSTANT_EXPRESSION,
	S_CONDITIONAL_EXPRESSION,
	S_INCLUSIVE_OR_EXPRESSION,
	S_LOGICAL_AND_EXPRESSION,
	S_LOGICAL_OR_EXPRESSION,
	S_EXCLUSIVE_OR_EXPRESSION,
	S_AND_EXPRESSION,
	S_EQUALITY_EXPRESSION,
	S_RELATIONAL_EXPRESSION,
	S_SHIFT_EXPRESSION,
	S_ADDITIVE_EXPRESSION,
	S_MULTIPLICATIVE_EXPRESSION,
	S_CAST_EXPRESSION,
	S_UNARY_EXPRESSION,
	S_UNARY_OPERATOR,
	S_ENUMERATION_CONSTANT,
	S_ASSIGNMENT_OPERATOR,
	S_SPECIFIER_QUALIFIER_LIST,
	S_TYPE_SPECIFIER,
	S_ABSTRACT_DECLARATOR,
	S_DIRECT_ABSTRACT_DECLARATOR,
	S_PARAMETER_TYPE_LIST,
	S_PARAMETER_LIST,
	S_TYPE_QUALIFIER,
	S_STRUCT_OR_UNION_SPECIFIER,
	S_ENUM_SPECIFIER,
	S_TYPEDEF_NAME,
	S_POINTER,
	S_PARAMETER_DECLARATION,
	S_STRUCT_OR_UNION,
	S_STRUCT_DECLARATION_LIST,
	S_ENUMERATOR_LIST,
	S_TYPE_QUALIFIER_LIST,
	S_DECLARATION_SPECIFIERS,
	S_DECLARATOR,
	S_STRUCT_DECLARATION,
	S_ENUMERATOR,
	S_STORAGE_CLASS_SPECIFIER,
	S_FUNCTION_SPECIFIER,
	S_DIRECT_DECLARATOR,
	S_STRUCT_DECLARATOR_LIST,
	S_IDENTIFIER_LIST,
	S_STRUCT_DECLARATOR,
	
	S_MAX

} syntax_type_t;

struct tinfo_s;

//One node of abstract syntax tree
typedef struct syntax_node_s
{
	//Type of syntax element (or token) at this node
	syntax_type_t type;
	
	//Which option from the syntax_options table was used to construct this element
	char option;
	
	//Token that starts this element
	tok_t *start;
	
	//Token that ends this element
	tok_t *end;
	
	//Total number of tokens used in matching this element
	int ntoks;
	
	//Child syntax nodes (unless this is a token)
	struct syntax_node_s *children[SYNTAX_FANOUT];
	
	//Type information if relevant for this syntax
	struct tinfo_s *tinfo;
	
	//Constant value if relevant for this syntax
	void *value;
	
} syntax_node_t;

//Tries to build a syntax element. Returns the element (a tree) on success, or NULL on failure.
syntax_node_t *syntax_try(syntax_type_t type, tok_t *start, tok_t *end);

//Deletes a syntax free
void syntax_free(syntax_node_t *node);

//Tries to evaluate constant values for the given syntax element.
void syntax_doconst(syntax_node_t *node);

#endif //SYNTAX_H
