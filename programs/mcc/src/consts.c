//consts.c
//Type and value determination for constant syntax nodes
//Bryan E. Topp <betopp@betopp.com> 2021

#include "consts.h"
#include "alloc.h"
#include "tinfo.h"
#include "syntax.h"

#include <assert.h>
#include <stdlib.h>

void consts_intc(syntax_node_t *node)
{
	//Node is an integer constant token that needs type/value determination
	assert(node->type == (int)TOK_INTC);
	assert(node->tinfo == NULL);
	assert(node->value == NULL);
	
	//Set up type info for an integer constant.
	node->tinfo = alloc_mandatory(sizeof(tinfo_t));
	node->tinfo->cat = TINFO_BTYPE;
	
	//The way we parse, constants don't include a leading "-". So any constant is positive.
	//Evaluate as longest possible unsigned for now, and figure out where the number ends.
	unsigned long long int val = 0;
	int base = 10; //Default to base-10 constants
	char *remain = node->start->text;
	if(remain[0] == '0' && (remain[1] == 'x' || remain[1] == 'X'))
	{
		//Starts with 0x or 0X. Skip the prefix and parse as hex.
		base = 16;
		remain += 2;
	}
	else if(remain[0] == '0')
	{
		//Starts with 0. Skip the prefix and parse as oct.
		base = 8;
		remain++;
	}

	//Parse all digits
	while(remain[0] != '\0')
	{
		int digval = 0;
		if(remain[0] >= 'a' && remain[0] <= 'f')
		{
			//Digit at least 10, represented by lowercase letter
			digval = 10 + remain[0] - 'a';
		}
		else if(remain[0] >= 'A' && remain[0] <= 'F')
		{
			//Digit at least 10, represented by uppercase letter
			digval = 10 + remain[0] - 'A';
		}
		else if(remain[0] >= '0' && remain[0] <= '9')
		{
			//Digit under 10, represented by numeral
			digval = remain[0] - '0';
		}
		else
		{
			//No further digits
			break;
		}
		
		if(digval >= base)
		{
			tok_err(node->start, "bad digit in integer constant");
		}
		
		//We're parsing left-to-right. So each new digit makes all priors 10x (or 8x or 16x) bigger.
		//Then, each digit is added with unit value - so the last ends up as 1s.
		unsigned long long int old_val = val;
		val = (val * base) + digval;
		
		//Each time we add a digit, the number should get bigger (unless it's 0 and we added another 0).
		if( (val < old_val) || (old_val != 0 && val == old_val) )
		{
			tok_err(node->start, "overflow evaluating integer constant");
		}
		
		remain++;
	}
	
	//Check suffixes after the number.
	bool suf_u = false;
	bool suf_l = false;
	bool suf_ll = false;
	while(remain[0] != '\0')
	{
		if(remain[0] == 'u' || remain[0] == 'U')
		{
			suf_u = true;
			remain++;
		}
		else if(remain[0] == 'l' || remain[0] == 'L')
		{
			if(remain[1] == remain[0])
			{
				suf_ll = true;
				remain += 2;
			}
			else
			{
				suf_l = true;
				remain++;
			}
		}
		else
		{
			tok_err(node->start, "junk after integer constant");
		}
	}
	
	//Based on the value represented and the suffixes used, reduce to the right type.
	
	//We'll use the smallest allowed type for representing the value.
	//Suffixes and numeric base change which ones we're allowed to try.
	bool try_int = true;
	bool try_unsigned_int = true;
	bool try_long_int = true;
	bool try_unsigned_long_int = true;
	bool try_long_long_int = true;
	bool try_unsigned_long_long_int = true;
	
	if(suf_u)
	{
		//Any base with "unsigned" suffix means exclusively unsigned.
		try_int = false;
		try_long_int = false;
		try_long_long_int = false;
	}
	else if(base == 10)
	{
		//Base-10 is exclusively signed, unless u-suffixed (other bases try both)
		try_unsigned_int = false;
		try_unsigned_long_int = false;
		try_unsigned_long_long_int = false;
	}
	
	if(suf_l)
	{
		//Single L suffix precludes less than "long int"
		try_int = false;
		try_unsigned_int = false;
	}
	
	if(suf_ll)
	{
		//Double L suffix precludes less than "long long int"
		try_int = false;
		try_unsigned_int = false;
		try_long_int = false;
		try_unsigned_long_int = false;
	}
		
	if(try_int)
	{
		int val_int = val;
		if(val_int == val)
		{
			node->tinfo->btype = BTYPE_INT;
			node->value = alloc_mandatory(sizeof(int));
			*(int*)(node->value) = val_int;
			return;
		}
	}
	
	if(try_unsigned_int)
	{
		unsigned int val_unsigned_int = val;
		if(val_unsigned_int == val)
		{
			node->tinfo->btype = BTYPE_UINT;
			node->value = alloc_mandatory(sizeof(unsigned int));
			*(unsigned int*)(node->value) = val_unsigned_int;
			return;
		}
	}
	
	if(try_long_int)
	{
		long int val_long_int = val;
		if(val_long_int == val)
		{
			node->tinfo->btype = BTYPE_LONGINT;
			node->value = alloc_mandatory(sizeof(long int));
			*(long int*)(node->value) = val_long_int;
			return;
		}
	}
	
	if(try_unsigned_long_int)
	{
		unsigned long int val_unsigned_long_int = val;
		if(val_unsigned_long_int == val)
		{
			node->tinfo->btype = BTYPE_ULONGINT;
			node->value = alloc_mandatory(sizeof(unsigned long int));
			*(unsigned long int*)(node->value) = val_unsigned_long_int;
			return;
		}
	}
	
	if(try_long_long_int)
	{
		long long int val_long_long_int = val;
		if(val_long_long_int == val)
		{
			node->tinfo->btype = BTYPE_LONGLONGINT;
			node->value = alloc_mandatory(sizeof(long long int));
			*(long long int*)(node->value) = val_long_long_int;
			return;
		}
	}
	
	if(try_unsigned_long_long_int)
	{
		unsigned long long int val_unsigned_long_long_int = val;
		if(val_unsigned_long_long_int == val)
		{
			node->tinfo->btype = BTYPE_ULONGLONGINT;
			node->value = alloc_mandatory(sizeof(unsigned long long int));
			*(unsigned long long int*)(node->value) = val_unsigned_long_long_int;
			return;
		}
	}
	
	tok_err(node->start, "failed to find type to contain integer value");
}

void consts_fltc(syntax_node_t *node)
{
	(void)node;
	abort();
}

