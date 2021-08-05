//tinfo.h
//Type information and conversion
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef TINFO_H
#define TINFO_H

#include "btype.h"

#include <stdbool.h>
#include <sys/types.h>

//Categories of type
typedef enum tinfo_cat_e
{
	TINFO_NONE = 0, //None/invalid
	TINFO_BTYPE,
	TINFO_STRUCT,
	TINFO_UNION,
	TINFO_FUNC,
	TINFO_ARRAY,
	TINFO_POINTER,
	
	TINFO_MAX
	
} tinfo_cat_t;

typedef struct tinfo_s
{
	//Category of this type
	tinfo_cat_t cat;
	
	//If this is a basic type, which basic type
	btype_t btype;
	
	//If this contains other types (struct or union), the first of them.
	//If this is a pointer, the type to which it points.
	struct tinfo_s *content;
	
	//If this is a function, the parameter types
	struct tinfo_s *parms;
	
	//If this is a function, the return type
	struct tinfo_s *retval;
	
	//If this is an array, the size of the array.
	size_t elems;
	
	//If this type is part of a compound type, the next of the parent's children
	struct tinfo_s *next;
	
} tinfo_t;

//Returns true if the given data should be considered "nonzero" in the given type info
bool tinfo_val_nz(const tinfo_t *tinfo, const void *value);

//Returns true if the given data should be considered equal
bool tinfo_val_eq(const tinfo_t *tinfo_a, const void *value_a, const tinfo_t *tinfo_b, const void *value_b);

//Returns true if the first data compares less than the second
bool tinfo_val_lt(const tinfo_t *tinfo_a, const void *value_a, const tinfo_t *tinfo_b, const void *value_b);

//Returns true if the given type is an arithmetic type
bool tinfo_is_arith(const tinfo_t *tinfo);

//Returns true if the given type is an integer type
bool tinfo_is_integer(const tinfo_t *tinfo);

//Allocates and constructs type information for a given basic type.
tinfo_t *tinfo_for_basic(btype_t t);

//Allocates and constructs type information for the result of an arithmetic operation between the two types.
tinfo_t *tinfo_for_arithmetic(const tinfo_t *op_a, const tinfo_t *op_b);

#endif //TINFO_H
