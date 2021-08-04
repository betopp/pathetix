//mmlibc/include/time.h
//Timekeeping definitions for MMK's libc
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _TIME_H
#define _TIME_H

#include <mmbits/typedef_clock.h>
#include <mmbits/typedef_size.h>
#include <mmbits/typedef_time.h>
#include <mmbits/typedef_clockid.h>
#include <mmbits/typedef_timer.h>
#include <mmbits/typedef_locale.h>
#include <mmbits/typedef_pid.h>

struct sigevent; //The standard specifies that this type is incomplete.

#include <mmbits/struct_tm.h>
#include <mmbits/struct_timespec.h>
#include <mmbits/struct_itimerspec.h>
#include <mmbits/define_null.h>
#include <mmbits/define_clocks_per_sec.h>
#include <mmbits/define_clockids.h>
#include <mmbits/define_timer_flags.h>


clock_t clock(void);
int clock_getcpuclockid(pid_t pid, clockid_t *clockid);
int clock_getres(clockid_t clockid, struct timespec *res);
int clock_gettime(clockid_t clockid, struct timespec *tp);
int clock_nanosleep(clockid_t clockid, int flags, const struct timespec *rqtp, struct timespec *rmtp);
int clock_settime(clockid_t clockid, const struct timespec *tp);
double difftime(time_t time1, time_t time0);
struct tm *getdate(const char *string);
struct tm *gmtime(const time_t *timep);
struct tm *gmtime_r(const time_t *timep, struct tm *result);
struct tm *localtime(const time_t *timep);
struct tm *localtime_r(const time_t *timep, struct tm *result);
time_t mktime(struct tm *tm);
int nanosleep(const struct timespec *rqtp, struct timespec *rmtp);
size_t strftime(char *s, size_t max, const char *format, const struct tm *tm);
size_t strftime_l(char *s, size_t max, const char *format, const struct tm *tm, locale_t locale);
char *strptime(const char *s, const char *format, struct tm *tm);
time_t time(time_t *tloc);
int timer_create(clockid_t clockid, struct sigevent *evp, timer_t *timerid);
int timer_delete(timer_t timerid);
int timer_getoverrun(timer_t timerid);
int timer_gettime(timer_t timerid, struct itimerspec *value);
int timer_settime(timer_t timerid, int flags, const struct itimerspec *value, struct itimerspec *ovalue);
void tzset(void);

extern int daylight;
extern long timezone;
extern char *tzname[];

#endif //_TIME_H
