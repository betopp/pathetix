//rtc.c
//Realtime clock functions in libc
//Bryan E. Topp <betopp@betopp.com> 2021

#include <time.h>
#include <errno.h>
#include <px.h>

//Temp
char *tzname[2] = { "UTC", "UTC" };

time_t time(time_t *tloc)
{
	int64_t gps_usec = px_getrtc();
	if(gps_usec < 0)
	{
		//Todo - Janeway will need this to support negative time values.
		errno = -gps_usec;
		return (time_t)(-1);
	}
	
	int64_t gps_sec = gps_usec / 1000000l;
	int64_t unix_sec = gps_sec + 315964800l;
	if(tloc != NULL)
		*tloc = unix_sec;
	
	return unix_sec;
}

struct tm _localtime_buf;
struct tm *localtime(const time_t *clock)
{
	return localtime_r(clock, &_localtime_buf);
}

struct tm *localtime_r(const time_t *clock, struct tm *result)
{
	//This is really wrong but I don't care right now
	int64_t remain = *clock;
	
	result->tm_sec = remain % 60;
	remain /= 60;
	result->tm_min = remain % 60;
	remain /= 60;
	result->tm_hour = remain % 24;
	remain /= 24;
	result->tm_yday = remain % 365;
	remain /= 365;
	result->tm_year = remain + 70;	
	result->tm_mday = 0;
	result->tm_mon = 0;
	result->tm_wday = 0;
	result->tm_isdst = 0;
	result->tm_zone = tzname[0];
	result->tm_gmtoff = 0;

	return result;
}

time_t mktime(struct tm *tm)
{
	//Super temp too
	return (((((((tm->tm_year * 365) + tm->tm_yday) * 24) + tm->tm_hour) * 60) + tm->tm_min) * 60) + tm->tm_sec;
}

