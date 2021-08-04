//mmlibc/include/mmbits/struct_termios.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _STRUCT_TERMIOS_H
#define _STRUCT_TERMIOS_H

#include <mmbits/typedef_tcflag.h>
#include <mmbits/typedef_cc.h>

#define NCCS 32

struct termios
{
	speed_t c_ispeed;
	speed_t c_ospeed;
	tcflag_t c_iflag;
	tcflag_t c_oflag;
	tcflag_t c_cflag;
	tcflag_t c_lflag;
	cc_t c_cc[NCCS];
	int action; //Packed here for calling tcsetattr via ioctl.
};

#endif //_STRUCT_TERMIOS_H

