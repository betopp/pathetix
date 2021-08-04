//mmlibc/include/mmbits/struct_tm.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _STRUCT_TM_H
#define _STRUCT_TM_H

struct tm
{
	int tm_sec;   //Seconds [0,60]. 
	int tm_min;   //Minutes [0,59]. 
	int tm_hour;  //Hour [0,23]. 
	int tm_mday;  //Day of month [1,31]. 
	int tm_mon;   //Month of year [0,11]. 
	int tm_year;  //Years since 1900. 
	int tm_wday;  //Day of week [0,6] (Sunday =0). 
	int tm_yday;  //Day of year [0,365]. 
	int tm_isdst; //Daylight Savings flag. 
	char *tm_zone; //Timezone name
	long tm_gmtoff; //Offset from UTC in seconds
};

#endif //_STRUCT_TM_H
