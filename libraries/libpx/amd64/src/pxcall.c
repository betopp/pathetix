//pxcall.c
//Entry points for system-calls
//Bryan E. Topp <betopp@betopp.com> 2021

#include "px.h"

//Actual system-call triggers, for each number of parameters, defined in syscall.asm
extern uint64_t _pxcall0(uint64_t call);
extern uint64_t _pxcall1(uint64_t call, uint64_t p1);
extern uint64_t _pxcall2(uint64_t call, uint64_t p1, uint64_t p2);
extern uint64_t _pxcall3(uint64_t call, uint64_t p1, uint64_t p2, uint64_t p3);
extern uint64_t _pxcall4(uint64_t call, uint64_t p1, uint64_t p2, uint64_t p3, uint64_t p4);
extern uint64_t _pxcall5(uint64_t call, uint64_t p1, uint64_t p2, uint64_t p3, uint64_t p4, uint64_t p5);

//Use macro-trick to define system-call entries.
#define PXCALL0R(num, rt, name) \
	rt name(void) { \
		return (rt)_pxcall0(num); }
#define PXCALL0V(num, rt, name) \
	rt name(void) { \
		_pxcall0(num); }
	
#define PXCALL1R(num, rt, name, p1t) \
	rt name(p1t p1) { \
		return (rt)_pxcall1(num, (uint64_t)p1); }
#define PXCALL1V(num, rt, name, p1t) \
	rt name(p1t p1) { \
		_pxcall1(num, (uint64_t)p1); }
	
#define PXCALL2R(num, rt, name, p1t, p2t) \
	rt name(p1t p1, p2t p2) { \
		return (rt)_pxcall2(num, (uint64_t)p1, (uint64_t)p2); }
#define PXCALL2V(num, rt, name, p1t, p2t) \
	rt name(p1t p1, p2t p2) { \
		_pxcall2(num, (uint64_t)p1, (uint64_t)p2); }
	
#define PXCALL3R(num, rt, name, p1t, p2t, p3t) \
	rt name(p1t p1, p2t p2, p3t p3) { \
		return (rt)_pxcall3(num, (uint64_t)p1, (uint64_t)p2, (uint64_t)p3); }
#define PXCALL3V(num, rt, name, p1t, p2t, p3t) \
	rt name(p1t p1, p2t p2, p3t p3) { \
		_pxcall3(num, (uint64_t)p1, (uint64_t)p2, (uint64_t)p3); }
	
#define PXCALL4R(num, rt, name, p1t, p2t, p3t, p4t) \
	rt name(p1t p1, p2t p2, p3t p3, p4t p4) { \
		return (rt)_pxcall4(num, (uint64_t)p1, (uint64_t)p2, (uint64_t)p3, (uint64_t)p4); }
#define PXCALL4V(num, rt, name, p1t, p2t, p3t, p4t) \
	rt name(p1t p1, p2t p2, p3t p3, p4t p4) { \
		_pxcall4(num, (uint64_t)p1, (uint64_t)p2, (uint64_t)p3, (uint64_t)p4); }

#define PXCALL5R(num, rt, name, p1t, p2t, p3t, p4t, p5t) \
	rt name(p1t p1, p2t p2, p3t p3, p4t p4, p5t p5) { \
		return (rt)_pxcall5(num, (uint64_t)p1, (uint64_t)p2, (uint64_t)p3, (uint64_t)p4, (uint64_t)p5); }
#define PXCALL5V(num, rt, name, p1t, p2t, p3t, p4t, p5t) \
	rt name(p1t p1, p2t p2, p3t p3, p4t p4, p5t p5) { \
		_pxcall5(num, (uint64_t)p1, (uint64_t)p2, (uint64_t)p3, (uint64_t)p4, (uint64_t)p5); }

#include "../../pxcall.h"
