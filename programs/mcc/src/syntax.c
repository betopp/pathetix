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

//Printable names of syntax elements
static const char * syntax_names[S_MAX] = 
{
	[S_PRIMARY_EXPRESSION] = "primary-expression",
	[S_CONSTANT] = "constant",
	[S_EXPRESSION] = "expression",
	[S_POSTFIX_EXPRESSION] = "postfix-expression",
	[S_ARGUMENT_EXPRESSION_LIST] = "argument-expression-list",
	[S_TYPE_NAME] = "type-name",
	[S_INITIALIZER_LIST] = "initializer-list",
	[S_ASSIGNMENT_EXPRESSION] = "assignment-expression",
	[S_CONSTANT_EXPRESSION] = "constant-expression",
	[S_CONDITIONAL_EXPRESSION] = "conditional-expression",
	[S_INCLUSIVE_OR_EXPRESSION] = "inclusive-or-expression",
	[S_LOGICAL_AND_EXPRESSION] = "logical-and-expression",
	[S_LOGICAL_OR_EXPRESSION] = "logical-or-expression",
	[S_EXCLUSIVE_OR_EXPRESSION] = "exclusive-or-expression",
	[S_AND_EXPRESSION] = "and-expression",
	[S_EQUALITY_EXPRESSION] = "equality-expression",
	[S_RELATIONAL_EXPRESSION] = "relational-expression",
	[S_SHIFT_EXPRESSION] = "shift-expression",
	[S_ADDITIVE_EXPRESSION] = "additive-expression",
	[S_MULTIPLICATIVE_EXPRESSION] = "multiplicative-expression",
	[S_CAST_EXPRESSION] = "cast-expression",
	[S_UNARY_EXPRESSION] = "unary-expression",
	[S_UNARY_OPERATOR] = "unary-operator",
	[S_ENUMERATION_CONSTANT] = "enumeration-constant",
	[S_ASSIGNMENT_OPERATOR] = "assignment-operator",
	[S_SPECIFIER_QUALIFIER_LIST] = "specifier-qualifier-list",
	[S_TYPE_SPECIFIER] = "type-specifier",
	[S_ABSTRACT_DECLARATOR] = "abstract-declarator",
	[S_DIRECT_ABSTRACT_DECLARATOR] = "direct-abstract-declarator",
	[S_PARAMETER_TYPE_LIST] = "parameter-type-list",
	[S_PARAMETER_LIST] = "parameter-list",
	[S_TYPE_QUALIFIER] = "type-qualifier",
	[S_STRUCT_OR_UNION_SPECIFIER] = "struct-or-union-specifier",
	[S_ENUM_SPECIFIER] = "enum-specifier",
	[S_TYPEDEF_NAME] = "typedef-name",
	[S_POINTER] = "pointer",
	[S_PARAMETER_DECLARATION] = "parameter-declaration",
	[S_STRUCT_OR_UNION] = "struct-or-union",
	[S_STRUCT_DECLARATION_LIST] = "struct-declaration-list",
	[S_ENUMERATOR_LIST] = "enumerator-list",
	[S_TYPE_QUALIFIER_LIST] = "type-qualifier-list",
	[S_DECLARATION_SPECIFIERS] = "declaration-specifiers",
	[S_DECLARATOR] = "declarator",
	[S_STRUCT_DECLARATION] = "struct-declaration",
	[S_ENUMERATOR] = "enumerator",
	[S_STORAGE_CLASS_SPECIFIER] = "storage-class-specifier",
	[S_FUNCTION_SPECIFIER] = "function-specifier",
	[S_DIRECT_DECLARATOR] = "direct-declarator",
	[S_STRUCT_DECLARATOR_LIST] = "struct-declarator-list",
	[S_IDENTIFIER_LIST] = "identifier-list",
	[S_STRUCT_DECLARATOR] = "struct-declarator",
	[S_TRANSLATION_UNIT] = "translation-unit",
	[S_EXTERNAL_DECLARATION] = "external-declaration",
	[S_FUNCTION_DEFINITION] = "function-definition",
	[S_DECLARATION] = "declaration",
	[S_DECLARATION_LIST] = "declaration-list",
	[S_COMPOUND_STATEMENT] = "compound-statement",
	[S_INIT_DECLARATOR_LIST] = "init-declarator-list",
	[S_INIT_DECLARATOR] = "init-declarator",
	[S_INITIALIZER] = "initializer",
	[S_BLOCK_ITEM_LIST] = "block-item-list",
	[S_BLOCK_ITEM] = "block-item",
	[S_STATEMENT] = "statement",
	[S_LABELED_STATEMENT] = "labeled-statement",
	[S_EXPRESSION_STATEMENT] = "expression-statement",
	[S_SELECTION_STATEMENT] = "selection-statement",
	[S_ITERATION_STATEMENT] = "iteration-statement",
	[S_JUMP_STATEMENT] = "jump-statement",
	[S_DESIGNATION] = "designation",
	[S_DESIGNATOR_LIST] = "designator-list",
	[S_DESIGNATOR] = "designator",
};

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
	[S_TRANSLATION_UNIT] = 
	{
		{'e', S_EXTERNAL_DECLARATION, 0},
		{'t', S_TRANSLATION_UNIT, S_EXTERNAL_DECLARATION, 0},
		{0},
	},
	[S_EXTERNAL_DECLARATION] = 
	{
		{'f', S_FUNCTION_DEFINITION, 0},
		{'d', S_DECLARATION, 0},
		{0},
	},
	[S_FUNCTION_DEFINITION] = 
	{
		{'d', S_DECLARATION_SPECIFIERS, S_DECLARATOR, S_DECLARATION_LIST, S_COMPOUND_STATEMENT, 0},
		{'D', S_DECLARATION_SPECIFIERS, S_DECLARATOR, S_COMPOUND_STATEMENT, 0},
		{0},
	},
	[S_DECLARATION] = 
	{
		{'s', S_DECLARATION_SPECIFIERS, S_INIT_DECLARATOR_LIST, TOK_SCOLON, 0},
		{'S', S_DECLARATION_SPECIFIERS, TOK_SCOLON, 0},
		{0},
	},
	[S_DECLARATION_LIST] = 
	{
		{'d', S_DECLARATION, 0},
		{'l', S_DECLARATION_LIST, S_DECLARATION, 0},
		{0},
	},
	[S_COMPOUND_STATEMENT] = 
	{
		{'b', TOK_BRACEL, S_BLOCK_ITEM_LIST, TOK_BRACER, 0},
		{'B', TOK_BRACEL, TOK_BRACER, 0},
		{0}
	},
	[S_INIT_DECLARATOR_LIST] = 
	{
		{'i', S_INIT_DECLARATOR, 0},
		{'l', S_INIT_DECLARATOR_LIST, TOK_COMMA, S_INIT_DECLARATOR, 0},
		{0}
	},
	[S_INIT_DECLARATOR] = 
	{
		{'d', S_DECLARATOR, 0},
		{'e', S_DECLARATOR, TOK_EQU, S_INITIALIZER, 0},
		{0},
	},
	[S_INITIALIZER] = 
	{
		{'a', S_ASSIGNMENT_EXPRESSION, 0},
		{'b', TOK_BRACEL, S_INITIALIZER_LIST, TOK_BRACER, 0},
		{'c', TOK_BRACEL, S_INITIALIZER_LIST, TOK_COMMA, TOK_BRACER, 0},
		{0}
	},
	[S_BLOCK_ITEM_LIST] =
	{
		{'b', S_BLOCK_ITEM, 0},
		{'l', S_BLOCK_ITEM_LIST, S_BLOCK_ITEM, 0},
		{0},
	},
	[S_BLOCK_ITEM] = 
	{
		{'d', S_DECLARATION, 0},
		{'s', S_STATEMENT, 0},
		{0}
	},
	[S_STATEMENT] = 
	{
		{'l', S_LABELED_STATEMENT, 0},
		{'c', S_COMPOUND_STATEMENT, 0},
		{'e', S_EXPRESSION_STATEMENT, 0},
		{'s', S_SELECTION_STATEMENT, 0},
		{'i', S_ITERATION_STATEMENT, 0},
		{'j', S_JUMP_STATEMENT, 0},
		{0}
	},
	[S_LABELED_STATEMENT] = 
	{
		{'i', TOK_IDENT, TOK_COLON, S_STATEMENT, 0},
		{'c', TOK_CASE, S_CONSTANT_EXPRESSION, TOK_COLON, S_STATEMENT, 0},
		{'d', TOK_DEFAULT, TOK_COLON, S_STATEMENT, 0},
		{0}
	},
	[S_EXPRESSION_STATEMENT] = 
	{
		{'e', S_EXPRESSION, TOK_SCOLON, 0},
		{'E', TOK_SCOLON, 0},
		{0}
	},
	[S_SELECTION_STATEMENT] = 
	{
		{'i', TOK_IF, TOK_PARENL, S_EXPRESSION, TOK_PARENR, S_STATEMENT, 0},
		{'e', TOK_IF, TOK_PARENL, S_EXPRESSION, TOK_PARENR, S_STATEMENT, TOK_ELSE, S_STATEMENT, 0},
		{'s', TOK_SWITCH, TOK_PARENL, S_EXPRESSION, TOK_PARENR, S_STATEMENT, 0},
		{0}
	},
	[S_ITERATION_STATEMENT] = 
	{
		{'w', TOK_WHILE, TOK_PARENL, S_EXPRESSION, TOK_PARENR, S_STATEMENT, 0},
		{'d', TOK_DO, S_STATEMENT, TOK_WHILE, TOK_PARENL, S_EXPRESSION, TOK_PARENR, TOK_SCOLON, 0},
		{'f', TOK_FOR, TOK_PARENL, S_EXPRESSION, TOK_SCOLON, S_EXPRESSION, TOK_SCOLON, S_EXPRESSION, TOK_PARENR, S_STATEMENT, 0},
		{'g', TOK_FOR, TOK_PARENL,               TOK_SCOLON, S_EXPRESSION, TOK_SCOLON, S_EXPRESSION, TOK_PARENR, S_STATEMENT, 0},
		{'h', TOK_FOR, TOK_PARENL, S_EXPRESSION, TOK_SCOLON,               TOK_SCOLON, S_EXPRESSION, TOK_PARENR, S_STATEMENT, 0},
		{'i', TOK_FOR, TOK_PARENL,               TOK_SCOLON,               TOK_SCOLON, S_EXPRESSION, TOK_PARENR, S_STATEMENT, 0},
		{'j', TOK_FOR, TOK_PARENL, S_EXPRESSION, TOK_SCOLON, S_EXPRESSION, TOK_SCOLON,               TOK_PARENR, S_STATEMENT, 0},
		{'k', TOK_FOR, TOK_PARENL,               TOK_SCOLON, S_EXPRESSION, TOK_SCOLON,               TOK_PARENR, S_STATEMENT, 0},
		{'l', TOK_FOR, TOK_PARENL, S_EXPRESSION, TOK_SCOLON,               TOK_SCOLON,               TOK_PARENR, S_STATEMENT, 0},
		{'m', TOK_FOR, TOK_PARENL,               TOK_SCOLON,               TOK_SCOLON,               TOK_PARENR, S_STATEMENT, 0},
		{'n', TOK_FOR, TOK_PARENL, S_DECLARATION, S_EXPRESSION, TOK_SCOLON, S_EXPRESSION, TOK_PARENR, S_STATEMENT, 0},
		{'o', TOK_FOR, TOK_PARENL, S_DECLARATION,               TOK_SCOLON, S_EXPRESSION, TOK_PARENR, S_STATEMENT, 0},
		{'p', TOK_FOR, TOK_PARENL, S_DECLARATION, S_EXPRESSION, TOK_SCOLON,               TOK_PARENR, S_STATEMENT, 0},
		{'q', TOK_FOR, TOK_PARENL, S_DECLARATION,               TOK_SCOLON,               TOK_PARENR, S_STATEMENT, 0},
		{0}
	},
	[S_JUMP_STATEMENT] = 
	{
		{'g', TOK_GOTO, TOK_IDENT, TOK_SCOLON, 0},
		{'c', TOK_CONTINUE, TOK_SCOLON, 0},
		{'b', TOK_BREAK, TOK_SCOLON, 0},
		{'r', TOK_RETURN, S_EXPRESSION, TOK_SCOLON, 0},
		{'R', TOK_RETURN, TOK_SCOLON, 0},
		{0}
	},
	[S_INITIALIZER_LIST] = 
	{
		{'d', S_DESIGNATION, S_INITIALIZER, 0},
		{'D', S_INITIALIZER, 0},
		{'l', S_INITIALIZER_LIST, TOK_COMMA, S_DESIGNATION, S_INITIALIZER, 0},
		{'L', S_INITIALIZER_LIST, TOK_COMMA, S_INITIALIZER, 0},
		{0}
	},
	[S_DESIGNATION] = 
	{
		{'d', S_DESIGNATOR_LIST, TOK_EQU, 0},
		{0}
	},
	[S_DESIGNATOR_LIST] = 
	{
		{'d', S_DESIGNATOR, 0},
		{'l', S_DESIGNATOR_LIST, S_DESIGNATOR, 0},
		{0}
	},
	[S_DESIGNATOR] = 
	{
		{'b', TOK_BRACKL, S_CONSTANT_EXPRESSION, TOK_BRACKR, 0},
		{'d', TOK_DOT, TOK_IDENT, 0},
		{0}
	},
};

//First and follow sets for each syntax element
static uint8_t syntax_first[S_MAX][ (TOK_MAX + 7) / 8 ];
static uint8_t syntax_follow[S_MAX][ (TOK_MAX + 7) / 8 ];

//Propagates a first/follow set - elements from the "from" parameter are OR'd into the "into" parameter.
//Returns whether any changes were made.
static bool syntax_firstfollow_combine(uint8_t into[(TOK_MAX+7)/8], uint8_t from[(TOK_MAX+7)/8])
{
	bool retval = false;
	
	for(int tt = 0; tt < (TOK_MAX+7)/8; tt++)
	{
		uint8_t old = into[tt];
		into[tt] |= from[tt];
		if(into[tt] != old)
			retval = true;
	}
	
	return retval;
}

void syntax_init(void)
{
	//Init tables of which tokens are permissible in starting or following a syntax element.
	
	//Start by defining all tokens as only able to begin with themselves.
	for(int tt = TOK_NONE + 1; tt < TOK_MAX; tt++)
	{
		syntax_first[tt][tt / 8] = 1 << (tt % 8);
	}
	
	//Run through all options for all syntax elements.
	//If an element is seen following another, its "first" set is included in the other's "follow".
	//If an element is seen at the beginning of a larger element, its "first" is included in the outer's "first".
	//Do this iteratively until no sets are changed.
	while(1)
	{
		bool any_changed = false;
		for(int ss = S_START + 1; ss < S_MAX; ss++)
		{
			assert(syntax_options[ss][0][0] != 0);
			for(int oo = 0; syntax_options[ss][oo][0] != 0; oo++)
			{
				//Tokens that can begin the first subelement, can begin the overall element
				int first = syntax_options[ss][oo][1];
				assert(first != 0);
				any_changed = any_changed || syntax_firstfollow_combine(syntax_first[ss], syntax_first[first]);
				
				//Tokens that begin subsequent subelements can follow the prior subelement.
				for(int ee = 2; syntax_options[ss][oo][ee] != 0; ee++)
				{
					int prev_elem = syntax_options[ss][oo][ee-1];
					int this_elem = syntax_options[ss][oo][ee];
					bool ch = syntax_firstfollow_combine(syntax_follow[prev_elem], syntax_first[this_elem]);
					any_changed = any_changed || ch;
				}
				
				//Tokens that follow the larger element can follow its last subelement
				int last = 0;
				for(int ee = 1; syntax_options[ss][oo][ee] != 0; ee++)
				{
					last = syntax_options[ss][oo][ee];
				}
				assert(last != 0);
				any_changed = any_changed || syntax_firstfollow_combine(syntax_follow[last], syntax_follow[ss]);
			}
		}
		
		if(!any_changed)
			break;
	}
	
	//Print tables for checking
	printf("First:\n");
	for(int ss = S_START; ss < S_MAX; ss++)
	{
		printf("\t%s: ", syntax_names[ss]);
		for(int tt = 0; tt < TOK_MAX; tt++)
		{
			if(syntax_first[ss][tt/8] & (1 << (tt%8)))
			{
				printf("%s ", tok_typename(&(tok_t){.type = tt}));
			}
		}
		printf("\n");
	}
	printf("Follow:\n");
	for(int ss = S_START; ss < S_MAX; ss++)
	{
		printf("\t%s: ", syntax_names[ss]);
		for(int tt = 0; tt < TOK_MAX; tt++)
		{
			if(syntax_follow[ss][tt/8] & (1 << (tt%8)))
			{
				printf("%s ", tok_typename(&(tok_t){.type = tt}));
			}
		}
		printf("\n");
	}
}

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

syntax_node_t *syntax_try(syntax_type_t type, tok_t *start, tok_t *end)
{
	//Parser stack
	#define PSTACK_MAX 16
	syntax_node_t *pstack[PSTACK_MAX] = {NULL};
	int pstack_next = 0;
	
	//Attempt to consume all of the input
	tok_t *tt = start;
	while(1)
	{
		//If we've consumed all input, see if the parser stack can combine into the syntax we want.
		if(tt == NULL || tt == end->next)
		{
			//Check all options for the final syntax we're building
			for(int oo = 0; syntax_options[type][oo][0] != 0; oo++)
			{
				//Must have the right number of elements on the parser stack
				int total_elems = 0;
				for(int ee = 1; syntax_options[type][oo][ee] != 0; ee++)
				{
					total_elems = ee;
				}
				
				if(pstack_next != total_elems)
				{
					//Wrong number of elements parsed to match this.
					continue;
				}
				
				//Parser stack must match this definition of the syntax
				bool children_correct = true;
				for(int ee = 1; syntax_options[type][oo][ee] != 0; ee++)
				{
					if(pstack[ee-1]->type != syntax_options[type][oo][ee])
					{
						children_correct = false;
						break;
					}						
				}
				
				if(!children_correct)
				{
					//Wrong sequence of child elements on stack
					continue;
				}
				
				//Alright, looks like we can build the syntax we wanted.
				syntax_node_t *n = alloc_mandatory(sizeof(syntax_node_t));
				n->type = type;
				n->option = syntax_options[type][oo][0];
				n->start = pstack[0]->start;
				for(int cc = 0; cc < pstack_next; cc++)
				{
					n->children[cc] = pstack[cc];
					n->end = pstack[cc]->end;
					n->ntoks += pstack[cc]->ntoks;
				}
				return n;
			}
		}
		
		//See if anything on the parser stack can be combined into larger syntax elements...
		int found_ss = -1; //Resultant syntax that we found
		int found_oo = -1; //Option of how we'll construct that syntax
		int found_nelems = -1; //How many elements are used in the found syntax option
		for(int ss = S_START + 1; ss < S_MAX; ss++)
		{
			//Consider combining our existing stack entries to make syntax element ss.
			//First, look ahead and see if ss is allowed to be followed by the next input token.
			//(If we combine the existing stack into an element ss, then it ends up followed by remaining input.)
			if(tt != NULL)
			{
				int lookahead = tt->type;
				if((syntax_follow[ss][lookahead / 8] & (1 << (lookahead % 8))) == 0)
				{
					//Syntax element ss is not allowed to be followed by what follows in our input.
					//So we can't combine and form one right now.
					continue;
				}
			}
			
			assert(syntax_options[ss][0][0] != 0);
			for(int oo = 0; syntax_options[ss][oo][0] != 0; oo++)
			{
				//Figure out how many elements this option contains
				int nelems = -1;
				for(int ee = 1; syntax_options[ss][oo][ee] != 0; ee++)
				{
					nelems = ee;
				}
				assert(nelems >= 1);
				
				//If we don't have that many elements in our parser stack, we can't match it
				if(nelems > pstack_next)
					continue;
				
				//Compare the most recent parser stack elements with the required elements
				int comp_start = pstack_next - nelems;
				bool match = true;
				for(int cc = 0; cc < nelems; cc++)
				{
					if(pstack[comp_start + cc]->type != syntax_options[ss][oo][1 + cc])
					{
						match = false;
						break;
					}
				}
				
				if(match)
				{
					//Most recent entries on parser stack can be combined into this new element.
					//Pick the longest option that applies to the syntax element generated.
					if(nelems > found_nelems)
					{
						found_ss = ss;
						found_oo = oo;
						found_nelems = nelems;
					}
				}
			}
		}

		if(found_ss != -1)
		{
			//Found a larger syntax element we can make from the elements currently on the parser stack.
			syntax_node_t *n = alloc_mandatory(sizeof(syntax_node_t));
			n->type = found_ss;
			n->option = syntax_options[found_ss][found_oo][0];
			
			//Pop the elements off the stack and use them in constructing the new item.
			int lastelem = found_nelems - 1;
			for(int cc = 0; cc < found_nelems; cc++)
			{
				assert(pstack_next > 0);
				assert(syntax_options[found_ss][found_oo][1 + lastelem - cc] == pstack[pstack_next - 1]->type);
				
				n->children[ lastelem - cc ] = pstack[pstack_next - 1];
				n->ntoks += n->children[ lastelem - cc ]->ntoks;
				pstack_next--;
			}
			n->start = n->children[0]->start;
			n->end = n->children[lastelem]->end;
			
			assert(pstack_next >= 0);
			assert(pstack_next < PSTACK_MAX);
			
			//Put the new item on the stack instead
			pstack[pstack_next] = n;
			pstack_next++;
			
			//Try again to combine
			continue;
		}
		
		//Nothing on the stack can be combined into a larger syntax element.
		
		//See if we're out of input.
		if(tt == NULL || end->next == tt)
			break;
		
		//Consume another input token and put it onto the stack by itself.
		syntax_node_t *tn = alloc_mandatory(sizeof(syntax_node_t));
		tn->type = (syntax_type_t)(tt->type);
		tn->start = tt;
		tn->end = tt;
		tn->ntoks = 1;
		
		assert(pstack_next < PSTACK_MAX);
		pstack[pstack_next] = tn;
		pstack_next++;
		
		tt = tt->next;
	}
	
	//Didn't match
	while(pstack_next > 0)
	{
		syntax_free(pstack[pstack_next - 1]);
		pstack_next--;
	}
	return NULL;
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
	
	//Handle parenthesized expressions
	if(node->type == S_PRIMARY_EXPRESSION)
	{
		//Child 0 and 2 are parens, 1 is a single expression
		assert(node->children[0]->type == (int)TOK_PARENL);
		assert(node->children[2]->type == (int)TOK_PARENR);
		syntax_doconst(node->children[1]);
		
		node->tinfo = node->children[1]->tinfo;
		node->value = node->children[1]->value;
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
			
			bool succeeds = false;
			switch((tok_type_t)(node->children[1]->type))
			{
				case TOK_DAMP:
					succeeds = a_nz && b_nz;
					break;
				case TOK_DBAR:
					succeeds = a_nz || b_nz;
					break;
				default:
					abort();
			}
			
			node->value = alloc_mandatory(sizeof(int));
			*(int*)(node->value) = succeeds ? 1 : 0;
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
	
	//Handle relations
	if(node->type == S_RELATIONAL_EXPRESSION)
	{
		//Child 0 and 2 are operands. Child 1 specifies type of operation.
		syntax_doconst(node->children[0]);
		syntax_doconst(node->children[2]);
		
		//Result of relation always has type int, and is constant if both subexpressions are
		node->tinfo = tinfo_for_basic(BTYPE_INT);
		if(node->children[0]->value != NULL && node->children[2]->value != NULL)
		{
			//Todo - this approach might not be correct for some bizarre floating-point relations.
			bool lt = tinfo_val_lt(
				node->children[0]->tinfo, node->children[0]->value,
				node->children[2]->tinfo, node->children[2]->value);
			bool eq = tinfo_val_eq(
				node->children[0]->tinfo, node->children[0]->value,
				node->children[2]->tinfo, node->children[2]->value);
		
			bool succeeds = false;
			switch((tok_type_t)(node->children[1]->type))
			{
				case TOK_LT:
					succeeds = lt;
					break;
				case TOK_GT:
					succeeds = !(lt || eq);
					break;
				case TOK_LEQ:
					succeeds = lt || eq;
					break;
				case TOK_GEQ:
					succeeds = !lt;
					break;
				default:
					abort();
			}
			
			node->value = alloc_mandatory(sizeof(int));
			*(int*)(node->value) = succeeds ? 1 : 0;
		}
		
		return;
	}
	
	//Handle arithmetic
	if(node->type == S_ADDITIVE_EXPRESSION)
	{
		//Child 0 and 2 are operands. Child 1 is + or -.
		syntax_doconst(node->children[0]);
		syntax_doconst(node->children[2]);
		
		//Arithmetic only happens between basic types
		if(node->children[0]->tinfo->cat != TINFO_BTYPE)
			tok_err(node->children[0]->start, "arithmetic on non-basic type");
		if(node->children[2]->tinfo->cat != TINFO_BTYPE)
			tok_err(node->children[2]->start, "arithmetic on non-basic type");
				
		//Result is of appropriate basic type
		btype_t rtype = btype_for_arithmetic(node->children[0]->tinfo->btype, node->children[2]->tinfo->btype);
		node->tinfo = tinfo_for_basic(rtype);
		
		//Result is const if both operands are
		if(node->children[0]->value != NULL && node->children[2]->value != NULL)
		{
			void *conv_a = btype_conv(node->children[0]->value, node->children[0]->tinfo->btype, rtype);
			void *conv_b = btype_conv(node->children[2]->value, node->children[2]->tinfo->btype, rtype);
			switch((tok_type_t)(node->children[1]->type))
			{
				case TOK_PLUS:
					node->value = btype_add(conv_a, conv_b, rtype);
					break;
				case TOK_MINUS:
					node->value = btype_sub(conv_a, conv_b, rtype);
					break;
				default: abort();
			}
			free(conv_a);
			free(conv_b);
		}
		
		return;
	}
		
	//Todo - handle other expressions
	abort();
	return;
}