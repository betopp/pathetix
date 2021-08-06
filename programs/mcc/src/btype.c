//btype.c
//Built-in types recognized in C compiler
//Bryan E. Topp <betopp@betopp.com> 2021

#include "btype.h"
#include "alloc.h"

#include <stdlib.h>

//Basic categorization of types
typedef enum btype_class_e
{
	BTYPE_CLASS_NONE = 0, //None/invalid
	
	BTYPE_CLASS_SSI, //Standard signed integer
	BTYPE_CLASS_SUI, //Standard unsigned integer
	BTYPE_CLASS_REAL, //Real floating
	BTYPE_CLASS_COMPLEX, //Complex floating
	
	BTYPE_CLASS_MAX
} btype_class_t;
static const btype_class_t btype_class_table[BTYPE_MAX] = 
{
	[BTYPE_BOOL] = BTYPE_CLASS_SUI,
	[BTYPE_CHAR] = BTYPE_CLASS_SSI,
	[BTYPE_SCHAR] = BTYPE_CLASS_SSI,
	[BTYPE_SHORTINT] = BTYPE_CLASS_SSI,
	[BTYPE_INT] = BTYPE_CLASS_SSI,
	[BTYPE_LONGINT] = BTYPE_CLASS_SSI,
	[BTYPE_LONGLONGINT] = BTYPE_CLASS_SSI,
	[BTYPE_UCHAR] = BTYPE_CLASS_SUI,
	[BTYPE_USHORTINT] = BTYPE_CLASS_SUI,
	[BTYPE_UINT] = BTYPE_CLASS_SUI,
	[BTYPE_ULONGINT] = BTYPE_CLASS_SUI,
	[BTYPE_ULONGLONGINT] = BTYPE_CLASS_SUI,
	[BTYPE_FLOAT] = BTYPE_CLASS_REAL,
	[BTYPE_DOUBLE] = BTYPE_CLASS_REAL,
	[BTYPE_LONGDOUBLE] = BTYPE_CLASS_REAL,
	[BTYPE_CFLOAT] = BTYPE_CLASS_COMPLEX,
	[BTYPE_CDOUBLE] = BTYPE_CLASS_COMPLEX,
	[BTYPE_CLONGDOUBLE] = BTYPE_CLASS_COMPLEX,	
};

bool btype_is_arith(btype_t btype)
{
	return btype_is_integer(btype) || btype_is_floating(btype);
}

bool btype_is_integer(btype_t btype)
{
	btype_class_t cl = btype_class_table[btype];
	if(cl == BTYPE_CLASS_SUI || cl == BTYPE_CLASS_SSI)
		return true;
	
	return false;
}

bool btype_is_floating(btype_t btype)
{
	btype_class_t cl = btype_class_table[btype];
	if(cl == BTYPE_CLASS_REAL || cl == BTYPE_CLASS_COMPLEX)
		return true;
	
	return false;
}

btype_t btype_for_arithmetic(btype_t a, btype_t b)
{
	//Oh fucking kill me
	
	if(a == BTYPE_CLONGDOUBLE || b == BTYPE_CLONGDOUBLE || a == BTYPE_LONGDOUBLE || b == BTYPE_LONGDOUBLE)
	{
		//At least one operand has long-double components.
		//Result is complex long-double if either is complex, real long-double otherwise.
		if(btype_class_table[a] == BTYPE_CLASS_COMPLEX || btype_class_table[b] == BTYPE_CLASS_COMPLEX)
			return BTYPE_CLONGDOUBLE;
		else
			return BTYPE_LONGDOUBLE;
	}
	
	if(a == BTYPE_CDOUBLE || b == BTYPE_CDOUBLE || a == BTYPE_DOUBLE || b == BTYPE_DOUBLE)
	{
		//At least one operand has long components.
		//Result is complex long if either is complex, real long otherwise.
		if(btype_class_table[a] == BTYPE_CLASS_COMPLEX || btype_class_table[b] == BTYPE_CLASS_COMPLEX)
			return BTYPE_CDOUBLE;
		else
			return BTYPE_DOUBLE;
	}

	if(a == BTYPE_CFLOAT || b == BTYPE_CFLOAT || a == BTYPE_FLOAT || b == BTYPE_FLOAT)
	{
		//At least one operand has float components.
		//Result is complex float if either is complex, real float otherwise.
		if(btype_class_table[a] == BTYPE_CLASS_COMPLEX || btype_class_table[b] == BTYPE_CLASS_COMPLEX)
			return BTYPE_CFLOAT;
		else
			return BTYPE_FLOAT;
	}
	
	//No floating-point types happening.
	//Perform integer promotions.
	if(a == BTYPE_BOOL || a == BTYPE_UCHAR || a == BTYPE_USHORTINT)
		a = BTYPE_UINT;
	
	if(b == BTYPE_BOOL || b == BTYPE_UCHAR || b == BTYPE_USHORTINT)
		b = BTYPE_UINT;
	
	if(a == BTYPE_CHAR || a == BTYPE_SCHAR || a == BTYPE_SHORTINT)
		a = BTYPE_INT;
	
	if(b == BTYPE_CHAR || b == BTYPE_SCHAR || b == BTYPE_SHORTINT)
		b = BTYPE_INT;
	
	//Pick the larger of the two results, using unsigned in tiebreakers.
	if(a == BTYPE_ULONGLONGINT || b == BTYPE_ULONGLONGINT)
		return BTYPE_ULONGLONGINT;
	
	if(a == BTYPE_LONGLONGINT || b == BTYPE_LONGLONGINT)
		return BTYPE_LONGLONGINT;
	
	if(a == BTYPE_ULONGINT || b == BTYPE_ULONGINT)
		return BTYPE_ULONGINT;
	
	if(a == BTYPE_LONGINT || b == BTYPE_LONGINT)
		return BTYPE_LONGINT;
	
	if(a == BTYPE_UINT || b == BTYPE_UINT)
		return BTYPE_UINT;
	
	return BTYPE_INT;
}

bool btype_nz(btype_t btype, const void *value)
{
	switch(btype)
	{
		//Use macro-trick to cast to the appropriate type and compare.
		#define BTYPE_MAC(etype, ctype) case etype: return ((*(const ctype*)value) != 0);
		#include "btype_mac.def"
		#undef BTYPE_MAC
		default: abort(); //todo
	}
}

bool btype_eq(btype_t btype, const void *value_a, const void *value_b)
{
	switch(btype)
	{
		//Use macro-trick to cast to the appropriate type and compare.
		#define BTYPE_MAC(etype, ctype) case etype: return ((*(const ctype*)value_a) == (*(const ctype*)value_b));
		#include "btype_mac.def"
		#undef BTYPE_MAC
		default: abort(); //todo		
	}
}

bool btype_lt(btype_t btype, const void *value_a, const void *value_b)
{
	switch(btype)
	{
		//Use macro-trick to cast to the appropriate type and compare.
		#define BTYPE_MAC_NOCOMPLEX
		#define BTYPE_MAC(etype, ctype) case etype: return ((*(const ctype*)value_a) < (*(const ctype*)value_b));
		#include "btype_mac.def"
		#undef BTYPE_MAC
		#undef BTYPE_MAC_NOCOMPLEX
		default: abort(); //todo	
	}
}

static void *btype_alloc(btype_t t)
{
	switch(t)
	{
		//Use macro-trick to allocate right amount of memory
		#define BTYPE_MAC(etype, ctype) case etype: return alloc_mandatory(sizeof(ctype));
		#include "btype_mac.def"
		#undef BTYPE_MAC
		default: abort();
	}
}

void *btype_conv(const void *value, btype_t from, btype_t to)
{
	void *retval = btype_alloc(to);
	
	unsigned long long val_u;
	_Complex long double val_d;
	switch(from)
	{
		//Use macro-trick to read value
		#define BTYPE_MAC(etype, ctype) \
			case etype: \
			val_u = (*(const ctype*)value);\
			val_d = (*(const ctype*)value);\
			break;
		
		#include "btype_mac.def"
		#undef BTYPE_MAC
		
		default: abort();
	}
	
	//Use macro-trick to write value
	//Handle floating/non-floating by overwriting result for non-floats
	switch(to)
	{
		#define BTYPE_MAC(etype, ctype) case etype: (*(ctype*)retval) = val_u; break;
		#include "btype_mac.def"
		#undef BTYPE_MAC
		default: abort();
	}
	
	switch(to)
	{
		#define BTYPE_MAC_NOFLOAT
		#define BTYPE_MAC(etype, ctype) case etype: (*(ctype*)retval) = val_d; break;
		#include "btype_mac.def"
		#undef BTYPE_MAC
		#undef BTYPE_MAC_NOFLOAT
		default: abort();
	}
	
	return retval;
}

void *btype_add(const void *v1, const void *v2, btype_t t)
{
	void *retval = btype_alloc(t);
	
	switch(t)
	{
		//Use macro-trick to cast to the appropriate type and operate
		#define BTYPE_MAC(etype, ctype) case etype: *(ctype*)retval = ((*(ctype*)v1) + (*(ctype*)v2)); break;
		#include "btype_mac.def"
		#undef BTYPE_MAC
		default: abort(); //todo		
	}
	
	return retval;
}

void *btype_sub(const void *v1, const void *v2, btype_t t)
{
	void *retval = btype_alloc(t);
	
	switch(t)
	{
		//Use macro-trick to cast to the appropriate type and operate
		#define BTYPE_MAC(etype, ctype) case etype: *(ctype*)retval = ((*(ctype*)v1) - (*(ctype*)v2)); break;
		#include "btype_mac.def"
		#undef BTYPE_MAC
		default: abort(); //todo		
	}
	
	return retval;
}

