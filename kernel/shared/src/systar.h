//systar.h
//Initial contents of RAMdisk as TAR image
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef SYSTAR_H
#define SYSTAR_H

//Unpacks the system TAR as loaded by the bootloader. Frees its memory if possible.
void systar_unpack(void);

#endif //SYSTAR_H

