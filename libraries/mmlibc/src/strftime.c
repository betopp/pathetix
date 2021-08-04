//strftime.c
//Time formatiing in libc
//Bryan E. Topp <betopp@betopp.com> 2021

#include <time.h>
#include <string.h>
#include <stdio.h>

//Todo - a bunch of this is wrong (strange week-of-year stuff particularly).
//Just making a first pass for now to get oksh linking.

//Outputs a character in formatting time
static void _strftime_ch(char **buf, size_t *maxsize, char ch)
{
	if(*maxsize < 1)
		return;
	
	**buf = ch;
	(*buf)++;
	(*maxsize)--;
}

//Outputs a string in formatting time
static void _strftime_str(char **buf, size_t *maxsize, const char *str)
{
	while(*str != '\0')
	{
		_strftime_ch(buf, maxsize, *str);
		str++;
	}
}

//Used in formats that print 1-digit numbers
static void _strftime_1d(char **buf, size_t *maxsize, int val)
{
	_strftime_ch(buf, maxsize, '0' + (val % 10));
}

//Used in formats that print 2-digit zero-padded numbers
static void _strftime_2dz(char **buf, size_t *maxsize, int val)
{
	_strftime_ch(buf, maxsize, '0' + ((val / 10) % 10));
	_strftime_ch(buf, maxsize, '0' + (val % 10));
}

//Used in formats that print 2-digit blank-padded numbers
static void _strftime_2ds(char **buf, size_t *maxsize, int val)
{
	if(val < 10)
		_strftime_ch(buf, maxsize, ' ');
	else
		_strftime_ch(buf, maxsize, '0' + ((val / 10) % 10));
	
	_strftime_ch(buf, maxsize, '0' + (val % 10));	
}

//Used in formats that print 4-digit zero-padded numbers
static void _strftime_4dz(char **buf, size_t *maxsize, int val)
{
	_strftime_ch(buf, maxsize, '0' + ((val / 1000) % 10));
	_strftime_ch(buf, maxsize, '0' + ((val / 100 ) % 10));
	_strftime_ch(buf, maxsize, '0' + ((val / 10  ) % 10));
	_strftime_ch(buf, maxsize, '0' + ((val / 1   ) % 10));
}

//Full weekday name
static void _strftime_A(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	const char *name;
	switch(timeptr->tm_wday)
	{
		case 0: name = "Sunday"; break;
		case 1: name = "Monday"; break;
		case 2: name = "Tuesday"; break;
		case 3: name = "Wednesday"; break;
		case 4: name = "Thursday"; break;
		case 5: name = "Friday"; break;
		case 6: name = "Saturday"; break;
		default: name = "Badidxday"; break;
	}
	_strftime_str(buf, maxsize, name);
}

//Abbreviated weekday name
static void _strftime_a(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	const char *name;
	switch(timeptr->tm_wday)
	{
		case 0: name = "Sun"; break;
		case 1: name = "Mon"; break;
		case 2: name = "Tue"; break;
		case 3: name = "Wed"; break;
		case 4: name = "Thu"; break;
		case 5: name = "Fri"; break;
		case 6: name = "Sat"; break;
		default: name = "Bad"; break;
	}
	_strftime_str(buf, maxsize, name);
}

//Full month name
static void _strftime_B(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	const char *name;
	switch(timeptr->tm_mon)
	{
		case 0: name = "January"; break;
		case 1: name = "February"; break;
		case 2: name = "March"; break;
		case 3: name = "April"; break;
		case 4: name = "May"; break;
		case 5: name = "June"; break;
		case 6: name = "July"; break;
		case 7: name = "August"; break;
		case 8: name = "September"; break;
		case 9: name = "October"; break;
		case 10: name = "November"; break;
		case 11: name = "December"; break;
		default: name = "Badidxber"; break;
	}
	_strftime_str(buf, maxsize, name);
}

//Abbreviated month name
static void _strftime_b(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	const char *name;
	switch(timeptr->tm_mon)
	{
		case 0: name = "Jan"; break;
		case 1: name = "Feb"; break;
		case 2: name = "Mar"; break;
		case 3: name = "Apr"; break;
		case 4: name = "May"; break;
		case 5: name = "Jun"; break;
		case 6: name = "Jul"; break;
		case 7: name = "Aug"; break;
		case 8: name = "Sep"; break;
		case 9: name = "Oct"; break;
		case 10: name = "Nov"; break;
		case 11: name = "Dec"; break;
		default: name = "Bad"; break;
	}
	_strftime_str(buf, maxsize, name);
}

//Year/100 as decimal "00"-"99"
static void _strftime_C(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	_strftime_2dz(buf, maxsize, timeptr->tm_year / 100);
}

//Day of month as decimal "01"-"31"
static void _strftime_d(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	_strftime_2dz(buf, maxsize, timeptr->tm_mday);
}

//Day of month as decimal " 1"-"31"
static void _strftime_e(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	_strftime_2ds(buf, maxsize, timeptr->tm_mday);
}

//Year as decimal number with century, using the year for week-of-year
static void _strftime_G(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	_strftime_4dz(buf, maxsize, timeptr->tm_year + 1900);
}

//Year for week-of-year without century "00"-"99"
static void _strftime_g(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	_strftime_2dz(buf, maxsize, timeptr->tm_year % 100);
}

//Hour as decimal "00" - "23"
static void _strftime_H(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	_strftime_2dz(buf, maxsize, timeptr->tm_hour);
}

//Same as %b
static void _strftime_h(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	_strftime_b(buf, maxsize, timeptr);
}

//Hour as decimal "01"-"12"
static void _strftime_I(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	int h = timeptr->tm_hour;
	if(h == 0)
		h = 12;
	else if(h > 12)
		h -= 12;
	
	_strftime_2dz(buf, maxsize, h);
}

//Day of year as decimal "001"-"366"
static void _strftime_j(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	_strftime_ch(buf, maxsize, '0' + (timeptr->tm_yday / 100));
	_strftime_ch(buf, maxsize, '0' + ((timeptr->tm_yday / 10) % 10));
	_strftime_ch(buf, maxsize, '0' + (timeptr->tm_yday % 10));
}

//Hour as decimal, padded with blank " 0"-"23"
static void _strftime_k(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	_strftime_2ds(buf, maxsize, timeptr->tm_hour);
}

//Hour as decimal, padded with blank " 1"-"12"
static void _strftime_l(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	int h = timeptr->tm_hour;
	if(h == 0)
		h = 12;
	else if(h > 12)
		h -= 12;
	
	_strftime_2ds(buf, maxsize, h);
}

//Minute as decimal "00"-"59"
static void _strftime_M(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	_strftime_2dz(buf, maxsize, timeptr->tm_min);
}

//Month as decimal "01"-"12"
static void _strftime_m(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	_strftime_2dz(buf, maxsize, timeptr->tm_mon+1);
}

//Newline
static void _strftime_n(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	(void)timeptr;
	_strftime_ch(buf, maxsize, '\n');
}

//AM/PM
static void _strftime_p(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	if(timeptr->tm_hour < 12)
		_strftime_str(buf, maxsize, "AM");
	else
		_strftime_str(buf, maxsize, "PM");
}

//Equivalent to %H:%M
static void _strftime_R(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	_strftime_H(buf, maxsize, timeptr);	
	_strftime_ch(buf, maxsize, ':');
	_strftime_M(buf, maxsize, timeptr);
}

//Second as decimal "00"-"60"
static void _strftime_S(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	_strftime_2dz(buf, maxsize, timeptr->tm_sec);
}

//Seconds since Unix epoch
static void _strftime_s(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	char tmpbuf[32];
	char *tmpbufpos = &(tmpbuf[sizeof(tmpbuf)-1]);
	struct tm timebuf = *timeptr;
	time_t tt = mktime(&timebuf);
	int sign = 1;
	if(tt < 0)
	{
		sign = -1;
		tt *= -1;
	}
	
	while(tt > 0)
	{
		*tmpbufpos = '0' + (tt % 10);
		tmpbufpos--;
		tt /= 10;
	}
	
	if(sign < 0)
	{
		*tmpbufpos = '-';
		tmpbufpos--;
	}
	
	_strftime_str(buf, maxsize, tmpbufpos + 1);
}

//Tab
static void _strftime_t(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	(void)timeptr;
	_strftime_ch(buf, maxsize, '\t');
}

//Week number of year as decimal "00"-"53", Sunday being beginning of week
static void _strftime_U(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	_strftime_2dz(buf, maxsize, timeptr->tm_yday / 7);
}

//Weekday as number "1"-"7"
static void _strftime_u(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	_strftime_1d(buf, maxsize, timeptr->tm_wday + 1);
}

//Week number of year as decimal "01"-"53", Monday as beginning of week
static void _strftime_V(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	_strftime_2dz(buf, maxsize, timeptr->tm_yday / 7);
}

//Week number of year "00"-"53", Monday as first day of week
static void _strftime_W(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	_strftime_2dz(buf, maxsize, timeptr->tm_yday / 7);
}

//Weekday as decimal "0"-"6"
static void _strftime_w(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	_strftime_1d(buf, maxsize, timeptr->tm_wday);
}

//Year with century as decimal
static void _strftime_Y(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	_strftime_4dz(buf, maxsize, timeptr->tm_year + 1900);
}

//Year without century as decimal "00"-"99"
static void _strftime_y(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	_strftime_2dz(buf, maxsize, timeptr->tm_year % 100);
}

//Time zone name
static void _strftime_Z(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	char *name = timeptr->tm_zone;
	if(name == NULL)
		name = "UTC";
		
	_strftime_str(buf, maxsize, name);
}

//Time zone offset from UTC
static void _strftime_z(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	int offval = timeptr->tm_gmtoff;
	int offsign = (offval < 0) ? -1 : 1;
	offval *= offsign;
	
	int min_off = (offval / 60) % 60;
	int hour_off = offval / 3600;
	
	_strftime_ch(buf, maxsize, (offsign < 0) ? '-' : '+' );
	_strftime_2dz(buf, maxsize, hour_off);		
	_strftime_2dz(buf, maxsize, min_off);
}

//National representation of the time
static void _strftime_X(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	_strftime_H(buf, maxsize, timeptr);
	_strftime_ch(buf, maxsize, ':');
	_strftime_M(buf, maxsize, timeptr);
	_strftime_ch(buf, maxsize, ':');
	_strftime_S(buf, maxsize, timeptr);
	_strftime_z(buf, maxsize, timeptr);
}

//National representation of the date
static void _strftime_x(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	_strftime_Y(buf, maxsize, timeptr);
	_strftime_ch(buf, maxsize, '-');
	_strftime_m(buf, maxsize, timeptr);
	_strftime_ch(buf, maxsize, '-');
	_strftime_d(buf, maxsize, timeptr);
}

//National representation of date and time
static void _strftime_plus(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	_strftime_x(buf, maxsize, timeptr);
	_strftime_X(buf, maxsize, timeptr);
}

//Time and date in local form
static void _strftime_c(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	_strftime_plus(buf, maxsize, timeptr);
}

//Equivalent to %m/%d/%y
static void _strftime_D(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	_strftime_m(buf, maxsize, timeptr);
	_strftime_ch(buf, maxsize, '/');
	_strftime_d(buf, maxsize, timeptr);
	_strftime_ch(buf, maxsize, '/');
	_strftime_y(buf, maxsize, timeptr);
}

//Equivalent to %Y-%m-%d
static void _strftime_F(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	_strftime_Y(buf, maxsize, timeptr);
	_strftime_ch(buf, maxsize, '-');
	_strftime_m(buf, maxsize, timeptr);
	_strftime_ch(buf, maxsize, '-');
	_strftime_d(buf, maxsize, timeptr);
}

//Equivalent to %H:%M:%S
static void _strftime_T(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	_strftime_H(buf, maxsize, timeptr);
	_strftime_ch(buf, maxsize, ':');
	_strftime_M(buf, maxsize, timeptr);
	_strftime_ch(buf, maxsize, ':');
	_strftime_S(buf, maxsize, timeptr);
}

//Equivalent to %e-%b-%Y
static void _strftime_v(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	_strftime_s(buf, maxsize, timeptr);
	_strftime_ch(buf, maxsize, '-');
	_strftime_b(buf, maxsize, timeptr);
	_strftime_ch(buf, maxsize, '-');
	_strftime_Y(buf, maxsize, timeptr);
}

//Equivalent to %I:%M:%S %p
static void _strftime_r(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	_strftime_I(buf, maxsize, timeptr);	
	_strftime_ch(buf, maxsize, ':');
	_strftime_M(buf, maxsize, timeptr);
	_strftime_ch(buf, maxsize, ':');
	_strftime_S(buf, maxsize, timeptr);
	_strftime_ch(buf, maxsize, ' ');
	_strftime_p(buf, maxsize, timeptr);
}

//Percent
static void _strftime_pct(char **buf, size_t *maxsize, const struct tm *timeptr)
{
	(void)timeptr;
	_strftime_ch(buf, maxsize, '%');
}

size_t strftime(char *buf, size_t maxsize, const char *format, const struct tm *timeptr)
{
	size_t origsize = maxsize;
	while(*format != '\0' && maxsize > 0)
	{
		//Anything that's not a format specifier gets copied literally
		if(*format != '%')
		{
			//Copy normal character
			_strftime_ch(&buf, &maxsize, *format);
			format++;
			continue;
		}
		
		//Alright, looking at a format specifier.
		//Advance past the percent sign.
		format++;
		if(*format == '\0')
			break; //Guard against bad input
		
		//Act on the letter following the percent.
		switch(*format)
		{
			case 'A': _strftime_A(&buf, &maxsize, timeptr); break;
			case 'a': _strftime_a(&buf, &maxsize, timeptr); break;
			case 'B': _strftime_B(&buf, &maxsize, timeptr); break;
			case 'b': _strftime_b(&buf, &maxsize, timeptr); break;
			case 'C': _strftime_C(&buf, &maxsize, timeptr); break;
			case 'c': _strftime_c(&buf, &maxsize, timeptr); break;
			case 'D': _strftime_D(&buf, &maxsize, timeptr); break;
			case 'd': _strftime_d(&buf, &maxsize, timeptr); break;
			case 'e': _strftime_e(&buf, &maxsize, timeptr); break;
			case 'F': _strftime_F(&buf, &maxsize, timeptr); break;
			case 'G': _strftime_G(&buf, &maxsize, timeptr); break;
			case 'g': _strftime_g(&buf, &maxsize, timeptr); break;
			case 'H': _strftime_H(&buf, &maxsize, timeptr); break;
			case 'h': _strftime_h(&buf, &maxsize, timeptr); break;
			case 'I': _strftime_I(&buf, &maxsize, timeptr); break;
			case 'j': _strftime_j(&buf, &maxsize, timeptr); break;
			case 'k': _strftime_k(&buf, &maxsize, timeptr); break;
			case 'l': _strftime_l(&buf, &maxsize, timeptr); break;
			case 'M': _strftime_M(&buf, &maxsize, timeptr); break;
			case 'm': _strftime_m(&buf, &maxsize, timeptr); break;
			case 'n': _strftime_n(&buf, &maxsize, timeptr); break;
			case 'p': _strftime_p(&buf, &maxsize, timeptr); break;
			case 'R': _strftime_R(&buf, &maxsize, timeptr); break;
			case 'r': _strftime_r(&buf, &maxsize, timeptr); break;
			case 'S': _strftime_S(&buf, &maxsize, timeptr); break;
			case 's': _strftime_s(&buf, &maxsize, timeptr); break;
			case 'T': _strftime_T(&buf, &maxsize, timeptr); break;
			case 't': _strftime_t(&buf, &maxsize, timeptr); break;
			case 'U': _strftime_U(&buf, &maxsize, timeptr); break;
			case 'u': _strftime_u(&buf, &maxsize, timeptr); break;
			case 'V': _strftime_V(&buf, &maxsize, timeptr); break;
			case 'v': _strftime_v(&buf, &maxsize, timeptr); break;
			case 'W': _strftime_W(&buf, &maxsize, timeptr); break;
			case 'w': _strftime_w(&buf, &maxsize, timeptr); break;
			case 'X': _strftime_X(&buf, &maxsize, timeptr); break;
			case 'x': _strftime_x(&buf, &maxsize, timeptr); break;
			case 'Y': _strftime_Y(&buf, &maxsize, timeptr); break;
			case 'y': _strftime_y(&buf, &maxsize, timeptr); break;
			case 'Z': _strftime_Z(&buf, &maxsize, timeptr); break;
			case 'z': _strftime_z(&buf, &maxsize, timeptr); break;
			case '+': _strftime_plus(&buf, &maxsize, timeptr); break;
			case '%': _strftime_pct(&buf, &maxsize, timeptr); break;
			default: break;
		}
		
		//Advance past the format specifier
		format++;
	}
	
	//There must still be room for the terminating NUL.
	if(maxsize < 1)
		return 0; //Overflow
	
	*buf = '\0';
	maxsize--;
	
	//Return how many characters were filled in, including the NUL.
	return origsize - maxsize;
}
