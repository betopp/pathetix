//ctype.c
//Implementation of character-type functions for libc
//Bryan E. Topp <betopp@betopp.com> 2021

#include <ctype.h>

//Last I checked, these return the same boolean result as glibc for 0...255.

int isalnum(int x)
{
	return isalpha(x) || isdigit(x);
}

int isalpha(int x)
{
	if(x >= 'A' && x <= 'Z')
		return 1;
	if(x >= 'a' && x <= 'z')
		return 1;
	
	return 0;
}

int isascii(int x)
{	
	if(x >= 0 && x < 128)
		return 1;
	else
		return 0;
}

int isblank(int x)
{
	if(x == ' ')
		return 1;
	if(x == '\t')
		return 1;
	
	return 0;
}

int iscntrl(int x)
{
	if(x >= 0 && x < 32)
		return 1;
	
	if(x == 127)
		return 1;
	
	return 0;
}

int isdigit(int x)
{
	if(x >= '0' && x <= '9')
		return 1;
	
	return 0;
}

int isgraph(int x)
{
	return isprint(x) && !isspace(x);
}

int islower(int x)
{
	if(x >= 'a' && x <= 'z')
		return 1;
	
	return 0;
}

int isprint(int x)
{
	return isascii(x) && !iscntrl(x);
}

int ispunct(int x)
{
	return isgraph(x) && !isalnum(x);
}

int isspace(int x)
{
	if(x == ' ') //Space
		return 1;
	if(x == '\f') //Form-feed
		return 1;
	if(x == '\n') //Newline
		return 1;
	if(x == '\r') //Carriage return
		return 1;
	if(x == '\t') //Horizontal Tab
		return 1;
	if(x == '\v') //Vertical tab
		return 1;
	
	return 0;
}

int isupper(int x)
{
	if(x >= 'A' && x <= 'Z')
		return 1;
	
	return 0;
}

int isxdigit(int x)
{
	if(x >= '0' && x <= '9')
		return 1;
	
	if(x >= 'a' && x <= 'f')
		return 1;
	
	if(x >= 'A' && x <= 'F')
		return 1;
	
	return 0;
}

int toascii(int x)
{
	return x & 0x7F;
}

int tolower(int x)
{
	if(isupper(x))
		return x + 'a' - 'A';
	
	return x;
}

int toupper(int x)
{
	if(islower(x))
		return x + 'A' - 'a';
	
	return x;
}

//Stubs for locale-specific checks.
//We don't support locales for now.

int isalnum_l(int x, locale_t l)
{
	(void)l;
	return isalnum(x);
}

int isalpha_l(int x, locale_t l)
{
	(void)l;
	return isalpha(x);
}

int isblank_l(int x, locale_t l)
{
	(void)l;
	return isblank(x);
}

int iscntrl_l(int x, locale_t l)
{
	(void)l;
	return iscntrl(x);
}

int isdigit_l(int x, locale_t l)
{
	(void)l;
	return isdigit(x);
}

int isgraph_l(int x, locale_t l)
{
	(void)l;
	return isgraph(x);
}

int islower_l(int x, locale_t l)
{
	(void)l;
	return islower(x);
}

int isprint_l(int x, locale_t l)
{
	(void)l;
	return isprint(x);
}

int ispunct_l(int x, locale_t l)
{
	(void)l;
	return ispunct(x);
}

int isspace_l(int x, locale_t l)
{
	(void)l;
	return isspace(x);
}

int isupper_l(int x, locale_t l)
{
	(void)l;
	return isupper(x);
}

int isxdigit_l(int x, locale_t l)
{
	(void)l;
	return isxdigit(x);
}

int tolower_l(int x, locale_t l)
{
	(void)l;
	return tolower(x);
}

int toupper_l(int x, locale_t l)
{
	(void)l;
	return toupper(x);
}
