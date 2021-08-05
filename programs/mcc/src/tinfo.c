//tinfo.c
//Type information and conversion
//Bryan E. Topp <betopp@betopp.com> 2021

#include "tinfo.h"
#include "btype.h"
#include "alloc.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

//Returns true if the given data should be considered "nonzero" in the given type info
bool tinfo_val_nz(const tinfo_t *tinfo, const void *value)
{
	if(tinfo->cat == TINFO_BTYPE)
	{
		return btype_nz(tinfo->btype, value);
	}
	abort(); //todo
}

bool tinfo_val_eq(const tinfo_t *tinfo_a, const void *value_a, const tinfo_t *tinfo_b, const void *value_b)
{
	if(tinfo_a->cat != tinfo_b->cat)
	{
		//todo
		abort();
	}
	
	if(tinfo_a->cat != TINFO_BTYPE)
	{
		//todo
		abort();
	}
	
	btype_t arith_btype = btype_for_arithmetic(tinfo_a->btype, tinfo_b->btype);
	
	void *a_conv = btype_conv(value_a, tinfo_a->btype, arith_btype);
	void *b_conv = btype_conv(value_b, tinfo_b->btype, arith_btype);
	bool result = btype_eq(arith_btype, a_conv, b_conv);
	free(a_conv);
	free(b_conv);
	
	return result;
}

bool tinfo_val_lt(const tinfo_t *tinfo_a, const void *value_a, const tinfo_t *tinfo_b, const void *value_b)
{
	if(tinfo_a->cat != tinfo_b->cat)
	{
		//todo
		abort();
	}
	
	if(tinfo_a->cat != TINFO_BTYPE)
	{
		//todo
		abort();
	}
	
	btype_t arith_btype = btype_for_arithmetic(tinfo_a->btype, tinfo_b->btype);
	
	void *a_conv = btype_conv(value_a, tinfo_a->btype, arith_btype);
	void *b_conv = btype_conv(value_b, tinfo_b->btype, arith_btype);
	bool result = btype_lt(arith_btype, a_conv, b_conv);
	free(a_conv);
	free(b_conv);
	
	return result;	
}

bool tinfo_is_arith(const tinfo_t *tinfo)
{
	if(tinfo->cat != TINFO_BTYPE)
		return false;
	
	return btype_is_arith(tinfo->btype);
}

bool tinfo_is_integer(const tinfo_t *tinfo)
{
	if(tinfo->cat != TINFO_BTYPE)
		return false;
	
	return btype_is_integer(tinfo->btype);
}

tinfo_t *tinfo_for_basic(btype_t t)
{
	tinfo_t *result = alloc_mandatory(sizeof(tinfo_t));
	result->cat = TINFO_BTYPE;
	result->btype = t;
	return result;
}

tinfo_t *tinfo_for_arithmetic(const tinfo_t *op_a, const tinfo_t *op_b)
{
	if(op_a->cat != TINFO_BTYPE || op_b->cat != TINFO_BTYPE)
	{
		//Can't perform arithmetic unless on basic types
		return NULL;
	}
	
	btype_t btype_result = btype_for_arithmetic(op_a->btype, op_b->btype);
	if(btype_result == BTYPE_NONE)
	{
		//Invalid combination of basic types for arithmetic
		return NULL;
	}
	
	return tinfo_for_basic(btype_result);
}

