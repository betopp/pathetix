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
bool tinfo_val_nz(const tinfo_t *tinfo, void *value)
{
	if(tinfo->cat == TINFO_BTYPE)
	{
		//Basic type.
		switch(tinfo->btype)
		{
			case BTYPE_BOOL:         return (*(bool*)value                  ) != 0;
			case BTYPE_CHAR:         return (*(char*)value                  ) != 0;
			case BTYPE_SCHAR:        return (*(signed char*)value           ) != 0;
			case BTYPE_SHORTINT:     return (*(short int*)value             ) != 0;
			case BTYPE_INT:          return (*(int*)value                   ) != 0;
			case BTYPE_LONGINT:      return (*(long int*)value              ) != 0;
			case BTYPE_LONGLONGINT:  return (*(long long int*)value         ) != 0;
			case BTYPE_UCHAR:        return (*(unsigned char*)value         ) != 0;
			case BTYPE_USHORTINT:    return (*(short int*)value             ) != 0;
			case BTYPE_UINT:         return (*(unsigned int*)value          ) != 0;
			case BTYPE_ULONGINT:     return (*(unsigned long int*)value     ) != 0;
			case BTYPE_ULONGLONGINT: return (*(unsigned long long int*)value) != 0;
			case BTYPE_FLOAT:        return (*(float*)value                 ) != 0;
			case BTYPE_DOUBLE:       return (*(double*)value                ) != 0;
			case BTYPE_LONGDOUBLE:   return (*(long double*)value           ) != 0;
			case BTYPE_CFLOAT:       return (*(_Complex float*)value        ) != 0;
			case BTYPE_CDOUBLE:      return (*(_Complex double*)value       ) != 0;
			case BTYPE_CLONGDOUBLE:  return (*(_Complex long double*)value  ) != 0;
			default: abort(); //todo
		}
	}
	abort(); //todo
}

bool tinfo_val_eq(const tinfo_t *tinfo_a, void *value_a, const tinfo_t *tinfo_b, void *value_b)
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
	
	if(tinfo_a->btype != tinfo_b->btype)
	{
		//todo
		abort();
	}
	
	switch(tinfo_a->btype)
	{
		case BTYPE_BOOL:         return (*(bool*)value_a                  ) == (*(bool*)value_b                  );
		case BTYPE_CHAR:         return (*(char*)value_a                  ) == (*(char*)value_b                  );
		case BTYPE_SCHAR:        return (*(signed char*)value_a           ) == (*(signed char*)value_b           );
		case BTYPE_SHORTINT:     return (*(short int*)value_a             ) == (*(short int*)value_b             );
		case BTYPE_INT:          return (*(int*)value_a                   ) == (*(int*)value_b                   );
		case BTYPE_LONGINT:      return (*(long int*)value_a              ) == (*(long int*)value_b              );
		case BTYPE_LONGLONGINT:  return (*(long long int*)value_a         ) == (*(long long int*)value_b         );
		case BTYPE_UCHAR:        return (*(unsigned char*)value_a         ) == (*(unsigned char*)value_b         );
		case BTYPE_USHORTINT:    return (*(short int*)value_a             ) == (*(short int*)value_b             );
		case BTYPE_UINT:         return (*(unsigned int*)value_a          ) == (*(unsigned int*)value_b          );
		case BTYPE_ULONGINT:     return (*(unsigned long int*)value_a     ) == (*(unsigned long int*)value_b     );
		case BTYPE_ULONGLONGINT: return (*(unsigned long long int*)value_a) == (*(unsigned long long int*)value_b);
		case BTYPE_FLOAT:        return (*(float*)value_a                 ) == (*(float*)value_b                 );
		case BTYPE_DOUBLE:       return (*(double*)value_a                ) == (*(double*)value_b                );
		case BTYPE_LONGDOUBLE:   return (*(long double*)value_a           ) == (*(long double*)value_b           );
		case BTYPE_CFLOAT:       return (*(_Complex float*)value_a        ) == (*(_Complex float*)value_b        );
		case BTYPE_CDOUBLE:      return (*(_Complex double*)value_a       ) == (*(_Complex double*)value_b       );
		case BTYPE_CLONGDOUBLE:  return (*(_Complex long double*)value_a  ) == (*(_Complex long double*)value_b  );
		default: abort(); //todo		
	}
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

