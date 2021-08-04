//excsig.c
//Exception mapping for AMD64
//Bryan E. Topp <betopp@betopp.com> 2021

//We do this in C so we can use signal numbers in signal.h
#include <signal.h>

//The input to this function is a vector number from the AMD64 (see: AMD doc 24593 section 8.2).
//The output is the signal number to give to the kernel (mostly like SysVABI)
int excsig(int exception)
{
	switch(exception)
	{
		case 0: return SIGFPE; // #DE - divide-by-zero
		case 1: return SIGTRAP; // #DB - debug
		case 2: return 0; // NMI - nonmaskable interrupt
		case 3: return SIGTRAP; // #BP - breakpoint
		case 4: return SIGSEGV; // #OF - overflow
		case 5: return 0; // #BR - bound range
		case 6: return SIGILL; // #UD - undefined opcode
		case 7: return SIGFPE; // #NM - device not available
		case 8: return 0; // #DF - double fault
		case 9: return SIGSEGV; // coprocessor segment overrun
		case 10: return 0; // #TS - invalid TSS
		case 11: return 0; // #NP - segment not present
		case 12: return SIGSEGV; // #SS - stack exception
		case 13: return SIGSEGV; // #GP - general protection exception
		case 14: return SIGSEGV; // #PF - page fault
		case 15: return 0; // unused
		case 16: return SIGFPE; // #x87 floating-point exception
		case 17: return 0; // #AC - alignment check
		case 18: return 0; // #MC - machine check
		case 19: return 0; // #XF - SIMD floating-point exception
		case 28: return 0; // #HV - hypervisor injection
		case 29: return 0; // #VC - VMM communication exception
		case 30: return 0; // #SX - security exception
		default: return SIGILL;
	}
}
