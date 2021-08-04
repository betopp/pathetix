//printf.c
//Definitions for printf family for libc
//Bryan E. Topp <betopp@betopp.com> 2021

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <errno.h>
#include <stddef.h>
#include <stdbool.h>
#include <sys/types.h>

//Flags supported in printf, so we can pass a list of flags
#define _PRINTF_FLAG_ALT_FORM     0x01
#define _PRINTF_FLAG_ZERO_PAD     0x02
#define _PRINTF_FLAG_LEFT_ADJUST  0x04
#define _PRINTF_FLAG_SIGN_SPACE   0x08
#define _PRINTF_FLAG_ALWAYS_SIGN  0x10
#define _PRINTF_FLAG_USE_GROUPING 0x20

//Implementation of conversion for printf family - strings. Returns number of characters printed.
int _printf_conv_s(FILE *stream, int flags, int minwidth, int precision, const char *val)
{
	(void)flags;
	(void)minwidth;
	(void)precision;
	
	int num = 0;
	while(*val != '\0')
	{
		fputc(*val, stream);
		val++;
		num++;
	}
	return num;
}

//Implementation of conversion for printf family - signed integers. Returns number of characters printed.
int _printf_conv_d(FILE *stream, int flags, int minwidth, int precision, long long int val)
{
	int retval = 0;
	
	//Convert the number and figure out how many digits are actually needed.
	//64 characters here should be enough for even 128-bit numbers and grouping/sign/etc.
	//Note, this will be reversed, with low digits first.
	bool negative = false;
	if(val < 0)
	{
		negative = true;
		val = val * -1; //Ugh okay technically this will overflow for ~0... gotta handle that somewhere... //todo
	}
	char numbuf[64] = {0};
	int numlen = 0;
	int numlen_needed = 0;
	long long int unit = 1;
	for(int dd = 0; dd < 48; dd++)
	{		
		//Add grouping character (",") every three digits (probably need locale support here...)
		if(flags & _PRINTF_FLAG_USE_GROUPING)
		{
			if((dd != 0) && ((dd % 3) == 0))
			{
				numbuf[numlen] = ',';
				numlen++;
			}
		}
		
		//Figure out the next highest digit
		long long int digit = (val / unit) % 10;
		numbuf[numlen] = '0' + digit;
		numlen++;
		
		//Track the largest nonzero digit (i.e. how many digits we can't truncate).
		//Or, if we have a "precision", we always have to reach that many digits.
		if(digit > 0)
			numlen_needed = numlen;
		if(dd < precision)
			numlen_needed = numlen;
		
		unit *= 10;
	}
	
	//Clip off unneeded zeroes.
	if(numlen_needed == 0)
		numlen_needed = 1;
	
	numlen = numlen_needed;
	
	//Put the sign on (at the end, as this buffer is reversed)
	if(negative)
	{
		numbuf[numlen] = '-';
		numlen++;
	}
	else if(flags & _PRINTF_FLAG_ALWAYS_SIGN)
	{
		numbuf[numlen] = '+';
		numlen++;
	}
	else if(flags & _PRINTF_FLAG_SIGN_SPACE)
	{
		numbuf[numlen] = ' ';
		numlen++;
	}
	
	//If the number is shorter than the field width, and we want padding on the left, output the padding
	if(!(flags & _PRINTF_FLAG_LEFT_ADJUST))
	{
		for(int pp = 0; pp < (minwidth - numlen); pp++)
		{
			fputc(' ', stream);
			retval++;
		}
	}
	
	//Output the number
	for(int pp = numlen - 1; pp >= 0; pp--)
	{
		fputc(numbuf[pp], stream);
		retval++;
	}
	
	//If the number is shorter than the field width, and we want padding on the right, output the padding
	if(flags & _PRINTF_FLAG_LEFT_ADJUST)
	{
		for(int pp = 0; pp < (minwidth - numlen); pp++)
		{
			fputc(' ', stream);
			retval++;
		}
	}
	
	return retval;
}

//Implementation of conversion for printf family - unsigned integers. Returns number of characters printed.
int _printf_conv_u(FILE *stream, int flags, int minwidth, int precision, char base, unsigned long long val)
{
	int retval = 0;
	
	//Figure out what numerical base we'll be using, based on the format specifier
	unsigned long long baseval;
	switch(base)
	{
		case 'o':
			baseval = 8;
			break;
		case 'x':
		case 'X':
			baseval = 16;
			break;
		default:
			baseval = 10;
			break;
	}
	
	//Convert the number and figure out how many digits are actually needed.
	//64 characters here should be enough for even 128-bit numbers and grouping/etc.
	//Note, this will be reversed, with low digits first.
	char numbuf[64] = {0};
	int numlen = 0;
	int numlen_needed = 0;
	unsigned long long unit = 1;
	for(int dd = 0; dd < 48; dd++)
	{		
		//Add grouping character (",") every three digits (probably need locale support here...)
		if(flags & _PRINTF_FLAG_USE_GROUPING)
		{
			if((dd != 0) && ((dd % 3) == 0))
			{
				numbuf[numlen] = ',';
				numlen++;
			}
		}
		
		//Figure out the next highest digit
		long long unsigned digit = (val / unit) % baseval;
		
		if(base == 'X')
		{
			if(digit >= 10)
				numbuf[numlen] = 'A' + (digit - 10);
			else
				numbuf[numlen] = '0' + digit;
		}
		else if(base == 'x')
		{
			if(digit >= 10)
				numbuf[numlen] = 'a' + (digit - 10);
			else
				numbuf[numlen] = '0' + digit;
		}
		else
		{
			numbuf[numlen] = '0' + digit;
		}
		
		numlen++;
		
		//Track the largest nonzero digit (i.e. how many digits we can't truncate).
		//Or, if we have a "precision", we always have to reach that many digits.
		if(digit > 0)
			numlen_needed = numlen;
		if(dd < precision)
			numlen_needed = numlen;
		
		unit *= baseval;
	}
	
	//Clip off unneeded zeroes.
	numlen = numlen_needed;
	
	//If the number is shorter than the field width, and we want padding on the left, output the padding
	if(!(flags & _PRINTF_FLAG_LEFT_ADJUST))
	{
		for(int pp = 0; pp < (minwidth - numlen); pp++)
		{
			fputc(' ', stream);
			retval++;
		}
	}
	
	//Output the number
	for(int pp = numlen - 1; pp >= 0; pp--)
	{
		fputc(numbuf[pp], stream);
		retval++;
	}
	
	//If the number is shorter than the field width, and we want padding on the right, output the padding
	if(flags & _PRINTF_FLAG_LEFT_ADJUST)
	{
		for(int pp = 0; pp < (minwidth - numlen); pp++)
		{
			fputc(' ', stream);
			retval++;
		}
	}
	
	return retval;	
}

//We implement the bulk of this family in vfprintf.
//Other printf-family functions eventually call vfprintf.
//vfprintf uses fputc, which handles all destinations we print to.
int vfprintf(FILE *stream, const char *format, va_list ap)
{
	//Return value should be the number of characters printed.
	size_t retval = 0;
	
	//Work through format string until we reach its NUL terminator.
	while(*format != '\0')
	{		
		//See what we're looking at, at the beginning of the remaining format-string.
		if(*format != '%')
		{
			//Not looking at a format string. Copy the input verbatim.
			fputc(*format, stream);
			retval++;
			format++;
			continue;
		}
		
		//Looking at a percent symbol, starting a conversion specification. Advance past the percent.
		format++;
		
		//Consume zero or more flags.
		int flags = 0;
		bool no_more_flags = false;
		while(!no_more_flags)
		{
			switch(*format)
			{
				case '#':
					flags |= _PRINTF_FLAG_ALT_FORM;
					format++;
					break;
				case '0':
					flags |= _PRINTF_FLAG_ZERO_PAD;
					format++;
					break;
				case '-':
					flags |= _PRINTF_FLAG_LEFT_ADJUST;
					format++;
					break;
				case ' ':
					flags |= _PRINTF_FLAG_SIGN_SPACE;
					format++;
					break;
				case '+':
					flags |= _PRINTF_FLAG_ALWAYS_SIGN;
					format++;
					break;
				case '\'':
					flags |= _PRINTF_FLAG_USE_GROUPING;
					format++;
					break;
				default:
					//Not a flag character - don't consume and stop looking.
					no_more_flags = true;
					break;					
			}
		}
		
		//Consume optional minimum field width.
		int minwidth = 0;
		while(*format >= '0' && *format <= '9')
		{
			minwidth *= 10;
			minwidth += *format - '0';
			format++;
		}
		
		//Consume optional precision.
		int precision = 0;
		if(*format == '.')
		{
			format++;
			while(*format >= '0' && *format <= '9')
			{
				precision *= 10;
				precision += *format - '0';
				format++;
			}
		}
					
		//Consume optional length modifier.
		char length_mod[2] = {0};
		bool no_more_length = false;
		while(!no_more_length)
		{
			switch(*format)
			{
				case 'h':
					if(length_mod[0] == 'h')
						length_mod[1] = 'h'; //Second occurrance
					
					length_mod[0] = 'h';
					format++;
					break;
				case 'l':
					if(length_mod[0] == 'l')
						length_mod[1] = 'l'; //Second occurrance
					
					length_mod[0] = 'l';
					format++;
					break;
				case 'L':
					length_mod[0] = 'L';
					format++;
					break;
				case 'j':
					length_mod[0] = 'j';
					format++;
					break;
				case 'z':
					length_mod[0] = 'z';
					format++;
					break;
				case 't':
					length_mod[0] = 't';
					format++;
					break;
				default:
					no_more_length = true;
					break;
			}
		}
		
		//Consume conversion specifier and act on conversion.
		long long int signed_val = 0;
		long long unsigned unsigned_val = 0;
		const char *str_val = 0;
		switch(*format)
		{
			//Signed integers
			case 'd':
			case 'i':
				if(length_mod[0] == 'h' && length_mod[1] == 'h')
				{
					//Converting %hhd, signed char.
					signed_val = (signed char)(va_arg(ap, int)); //Will have been promoted to int.
				}
				else if(length_mod[0] == 'h')
				{
					//Converting %hd, short int.
					signed_val = (short int)(va_arg(ap, int)); //Will have been promoted to int.
				}
				else if(length_mod[0] == 'l' && length_mod[1] == 'l')
				{
					//Converting %lld, long long int.
					signed_val = (long long int)(va_arg(ap, long long int));
				}
				else if(length_mod[0] == 'l')
				{
					//Converting %ld, long int.
					signed_val = (long int)(va_arg(ap, long int));
				}
				else if(length_mod[0] == 'j')
				{
					//Converting %jd, intmax_t.
					signed_val = (intmax_t)(va_arg(ap, intmax_t));
				}
				else if(length_mod[0] == 'z')
				{
					//Converting %zd, ssize_t.
					signed_val = (ssize_t)(va_arg(ap, ssize_t));
				}
				else if(length_mod[0] == 't')
				{
					//Converting %td, ptrdiff_t.
					signed_val = (ptrdiff_t)(va_arg(ap, ptrdiff_t));
				}
				else
				{						
					//Converting %d, int.
					signed_val = (int)(va_arg(ap, int));
				}
				
				retval += _printf_conv_d(stream, flags, minwidth, precision, signed_val);
				break;
				
			//Unsigned integers	
			case 'o':
			case 'u':
			case 'x':
			case 'X':
				if(length_mod[0] == 'h' && length_mod[1] == 'h')
				{
					//Converting %hhu, unsigned char.
					unsigned_val = (unsigned char)(va_arg(ap, unsigned int)); //Will have been promoted to int.
				}
				else if(length_mod[0] == 'h')
				{
					//Converting %hu, unsigned short.
					unsigned_val = (unsigned short)(va_arg(ap, unsigned int)); //Will have been promoted to int.
				}
				else if(length_mod[0] == 'l' && length_mod[1] == 'l')
				{
					//Converting %llu, unsigned long long.
					unsigned_val = (unsigned long long)(va_arg(ap, unsigned long long));
				}
				else if(length_mod[0] == 'l')
				{
					//Converting %lu, unsigned long.
					unsigned_val = (unsigned long)(va_arg(ap, unsigned long));
				}
				else if(length_mod[0] == 'j')
				{
					//Converting %ju, uintmax_t.
					unsigned_val = (uintmax_t)(va_arg(ap, intmax_t));
				}
				else if(length_mod[0] == 'z')
				{
					//Converting %zu, size_t.
					unsigned_val = (size_t)(va_arg(ap, ssize_t));
				}
				else if(length_mod[0] == 't')
				{
					//Converting %tu, ptrdiff_t... as unsigned? Is this meaningful?
					unsigned_val = (ptrdiff_t)(va_arg(ap, ptrdiff_t));
				}
				else
				{						
					//Converting %u, unsigned int.
					unsigned_val = va_arg(ap, unsigned int);
					
				}
				
				retval += _printf_conv_u(stream, flags, minwidth, precision, *format, unsigned_val);
				break;
			
			//Double-precision floating-point, scientific notation
			case 'e':
			case 'E':
				break;
			
			//Double-precision floating-point, regular notation
			case 'f':
			case 'F':
				break;
			
			//Double-precision floating-point, automatic notation selection
			case 'g':
			case 'G':
				break;
			
			//Double-precision floating-point, hexadecimal scientific notation
			case 'a':
			case 'A':
				break;
			
			//Character, or wide-character as multibyte
			case 'c':
				break;
			
			//String
			case 's':
				str_val = va_arg(ap, const char*);
				retval += _printf_conv_s(stream, flags, minwidth, precision, str_val);
				break;
			
			default:
				break;
			
		}
		format++;
		
	}
	
	return retval;
}


int printf(const char *format, ...)
{
	//Set up va_list and call "v" version.
	va_list args;
	va_start(args, format);
	int retval = vprintf(format, args);
	va_end(args);
	return retval;
}

int fprintf(FILE *stream, const char *format, ...)
{
	//Set up va_list and call "v" version.
	va_list args;
	va_start(args, format);
	int retval = vfprintf(stream, format, args);
	va_end(args);
	return retval;
}

int dprintf(int fd, const char *format, ...)
{
	//Set up va_list and call "v" version.
	va_list args;
	va_start(args, format);
	int retval = vdprintf(fd, format, args);
	va_end(args);
	return retval;
}

int sprintf(char *str, const char *format, ...)
{
	//Set up va_list and call "v" version.
	va_list args;
	va_start(args, format);
	int retval = vsprintf(str, format, args);
	va_end(args);
	return retval;
}

int snprintf(char *str, size_t size, const char *format, ...)
{
	//Set up va_list and call "v" version.
	va_list args;
	va_start(args, format);
	int retval = vsnprintf(str, size, format, args);
	va_end(args);
	return retval;
}

int vprintf(const char *format, va_list ap)
{
	//Equivalent to calling vfprintf and specifying stdout as the stream.
	return vfprintf(stdout, format, ap);
}

int vdprintf(int fd, const char *format, va_list ap)
{
	//Phony FILE for writing directly to a file descriptor.
	FILE fd_stream = {
		.streamtype = _FILE_STREAMTYPE_RAWFD,
		.fd = fd,
	};	
	return vfprintf(&fd_stream, format, ap);
}

int vsprintf(char *str, const char *format, va_list ap)
{
	//Just do a vsnprintf with an enormous length.
	return vsnprintf(str, SIZE_MAX-1, format, ap);
}

int vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
	//Phony FILE for writing to a bounded buffer.
	FILE strn_stream = {
		.streamtype = _FILE_STREAMTYPE_STRN,
		.buf_ptr = str,
		.buf_size = size,
		.buf_out = 1,
	};
	return vfprintf(&strn_stream, format, ap);
}

