//mmlibc/include/mmbits/struct_winsize.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef _STRUCT_WINSIZE_H
#define _STRUCT_WINSIZE_H

struct winsize
{
	unsigned short ws_row;
	unsigned short ws_col;
	unsigned short ws_xpixel;
	unsigned short ws_ypixel;
};

#endif //_STRUCT_WINSIZE_H

