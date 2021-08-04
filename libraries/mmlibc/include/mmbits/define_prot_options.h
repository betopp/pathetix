//mmlibc/include/mmbits/define_prot_options.h
//Fragment for building C standard headers.
//Bryan E. Topp <betopp@betopp.com> 2020
#ifndef _DEFINE_PROT_OPTIONS_H
#define _DEFINE_PROT_OPTIONS_H

//Note - opposite of Linux; lines up with file mode bits (RWX = 421)
#define PROT_READ 0x4
#define PROT_WRITE 0x2
#define PROT_EXEC 0x1
#define PROT_NONE 0x0

#endif //_DEFINE_PROT_OPTIONS_H
