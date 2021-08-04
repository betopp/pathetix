//mmlibc/include/mmbits/define_termomodes.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _DEFINE_TERMOMODES_H
#define _DEFINE_TERMOMODES_H

//Like Linux
#define OPOST  0x0001
#define ONLCR  0x0004
#define OCRNL  0x0008
#define ONOCR  0x0010
#define ONLRET 0x0020
#define OFDEL  0x0080
#define OFILL  0x0040
#define NLDLY  0x0100
#define NL0    0x0000
#define NL1    0x0100
#define CRDLY  0x0600
#define CR0    0x0000
#define CR1    0x0200
#define CR2    0x0400
#define CR3    0x0600
#define TABDLY 0x1800
#define TAB0   0x0000
#define TAB1   0x0800
#define TAB2   0x1000
#define TAB3   0x1800
#define BSDLY  0x2000
#define BS0    0x0000
#define BS1    0x2000
#define VTDLY  0x4000
#define VT0    0x0000
#define VT1    0x4000
#define FFDLY  0x8000
#define FF0    0x0000
#define FF1    0x8000


#endif //_DEFINE_TERMOMODES_H

