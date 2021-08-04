//btype.h
//Built-in types recognized in C compiler
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef BTYPE_H
#define BTYPE_H

#include <stdbool.h>

//Enumeration of all built-in types supported.
typedef enum btype_e
{
	BTYPE_NONE = 0, //none/invalid
	
	//Types mentioned in C99
	BTYPE_BOOL,
	BTYPE_CHAR,
	BTYPE_SCHAR,
	BTYPE_SHORTINT,
	BTYPE_INT,
	BTYPE_LONGINT,
	BTYPE_LONGLONGINT,
	BTYPE_UCHAR,
	BTYPE_USHORTINT,
	BTYPE_UINT,
	BTYPE_ULONGINT,
	BTYPE_ULONGLONGINT,
	BTYPE_FLOAT,
	BTYPE_DOUBLE,
	BTYPE_LONGDOUBLE,
	BTYPE_CFLOAT,
	BTYPE_CDOUBLE,
	BTYPE_CLONGDOUBLE,
	
	BTYPE_MAX
	
} btype_t;

//Returns true if the given basic type is an arithmetic type.
bool btype_is_arith(btype_t btype);

//Returns true if the given basic type is an integer type.
bool btype_is_integer(btype_t btype);

//Returns true if the given basic type is a floating type.
bool btype_is_floating(btype_t btype);

//Returns the basic type that should be used for arithmetic between the two operand basic-types.
//Returns BTYPE_NONE if incompatible.
btype_t btype_for_arithmetic(btype_t a, btype_t b);

#endif //BTYPE_H


