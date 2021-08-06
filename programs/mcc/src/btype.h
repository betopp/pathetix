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

//Given two values in the same basic type, returns whether they are equal.
bool btype_eq(btype_t btype, const void *value_a, const void *value_b);

//Given two values in the same basic type, returns whether the first is less than the second.
bool btype_lt(btype_t btype, const void *value_a, const void *value_b);

//Given a value in a basic type, returns whether it compares not equal to zero.
bool btype_nz(btype_t btype, const void *value);

//Converts a value from one basic type to another. Returns newly allocated memory containing the new value.
void *btype_conv(const void *value, btype_t from, btype_t to);

//Adds two values of a given basic type. Returns newly allocated memory containing the result.
void *btype_add(const void *v1, const void *v2, btype_t t);

//Subtracts two values of a given basic type. Returns newly allocated memory containing the result.
void *btype_sub(const void *v1, const void *v2, btype_t t);

#endif //BTYPE_H


