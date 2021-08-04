//strtol.c
//String to number parsing
//Bryan E. Topp <betopp@betopp.com> 2021

#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <errno.h>
#include <limits.h>

//Implementation of string-to-number parsing
long long _strtoll_internal(const char *s, char **end, int base, bool *overflow_out, bool *negative_out)
{	
	//In cases where nothing is converted, we need to return the original string.
	const char *s_orig = s;
	
	//Skip whitespace
	while(isspace(*s))
	{
		s++;
	}
	
	//Consume + or - if present
	long long sign = 1;
	if(*s == '+')
	{
		sign = 1;
		s++;
	}
	else if(*s == '-')
	{
		sign = -1;
		s++;
	}
	
	//Detect and skip hex prefix
	if(base == 0 || base == 16)
	{
		if( (s[0] == '0') && ((s[1] == 'x') || (s[1] == 'X')))
		{
			s += 2;
			base = 16;
		}
	}
	
	//Detect and skip octal prefix
	if(base == 0 || base == 8)
	{
		if(s[0] == '0')
		{
			s++;
			base = 8;
		}
	}
	
	//If hex or octal prefix wasn't present, "automatic" base becomes base-10.
	if(base == 0)
		base = 10;
	
	//Convert digits until a nonvalid digit is found
	long long retval = 0;
	bool overflow = false;
	bool any_digits = false;
	while(1)
	{
		char next = *s;		
		int next_val = -1;
		if(next >= 'A' && next <= 'Z')
			next_val = 10 + next - 'A';
		else if(next >= 'a' && next <= 'z')
			next_val = 10 + next - 'a';
		else if(next >= '0' && next <= '9')
			next_val = next - '0';
		
		if(next_val < 0 || next_val >= base)
		{
			//Found a character that doesn't convert in the given base.
			//(This includes *s == '\0' case)
			if(end != NULL)
				*end = (char*)s; //POSIX has this const-correctness problem.
			
			break;
		}
		
		//This is a numeral in our base. Add to the conversion.
		long long old_retval = retval;
		if(sign > 0)
		{
			retval *= base;
			retval += next_val;
			if(retval < old_retval) //Should never get smaller as we multiply by a positive and add
				overflow = true;
		}
		else
		{
			retval *= base;
			retval -= next_val;
			if(retval > old_retval) //Should never get bigger as we multiply by a positive and subtract
				overflow = true;
		}
		
		//Keep converting more characters
		any_digits = true;
		s++;
	}
	
	//If nothing was converted, return a pointer to the original string
	if(!any_digits)
	{
		if(end != NULL)
			*end = (char*)s_orig; //POSIX has this const-correctness problem.
		
		*overflow_out = false;
		*negative_out = false;
		errno = EINVAL;
		return 0;
	}

	//Output whether we overflowed or not, and return the final (possibly overflowed) value.
	*overflow_out = overflow;
	*negative_out = (sign < 0);
	return retval;
}

long strtol(const char *s, char **end, int base)
{
	if( (base < 2 || base > 36) && (base != 0) )
	{
		errno = EINVAL;
		return 0;
	}
	
	bool overflow = false;
	bool negative = false;
	long long result = _strtoll_internal(s, end, base, &overflow, &negative);
	if(overflow)
	{
		errno = ERANGE;
		if(negative)
			return LONG_MIN;
		else
			return LONG_MAX;
	}
	
	if(result > LONG_MAX)
	{
		errno = ERANGE;
		result = LONG_MAX;
	}
	if(result < LONG_MIN)
	{
		errno = ERANGE;
		result = LONG_MIN;
	}
	
	return result;
}

long long strtoll(const char *s, char **end, int base)
{
	if( (base < 2 || base > 36) && (base != 0) )
	{
		errno = EINVAL;
		return 0;
	}
	
	bool overflow = false;
	bool negative = false;
	long long result = _strtoll_internal(s, end, base, &overflow, &negative);
	if(overflow)
	{
		errno = ERANGE;
		if(negative)
			return LLONG_MIN;
		else
			return LLONG_MAX;
	}
	return result;
}

long atol(const char *s)
{
	return strtol(s, NULL, 10);
}

long long atoll(const char *s)
{
	return strtoll(s, NULL, 10);
}

int atoi(const char *s)
{
	return (int)atol(s);
}
