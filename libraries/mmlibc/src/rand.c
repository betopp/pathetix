//rand.c
//Random number generator for libc
//Bryan E. Topp <betopp@betopp.com> 2021

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

//LFSR113
static uint32_t _rand_z1;
static uint32_t _rand_z2;
static uint32_t _rand_z3;
static uint32_t _rand_z4;

void srand(unsigned seed)
{
	_rand_z1 = seed;
	while(_rand_z1 < 2)
		_rand_z1 += 1 + seed;
	
	_rand_z2 = seed;
	while(_rand_z2 < 8)
		_rand_z2 += 1 + seed;
	
	_rand_z3 = seed;
	while(_rand_z3 < 16)
		_rand_z3 += 1 + seed;
	
	_rand_z4 = seed;
	while(_rand_z4 < 128)
		_rand_z4 += 1 + seed;
}

uint32_t _rand_uint32(void)
{
	if(_rand_z1 == 0)
		srand(getpid());
	
	_rand_z1 = ((_rand_z1 & 0xFFFFFFFEu) << 18) ^ (((_rand_z1 <<  6) ^ _rand_z1) >> 13);
	_rand_z2 = ((_rand_z2 & 0xFFFFFFF8u) <<  2) ^ (((_rand_z2 <<  2) ^ _rand_z2) >> 27);
	_rand_z3 = ((_rand_z3 & 0xFFFFFFF0u) <<  7) ^ (((_rand_z3 << 13) ^ _rand_z3) >> 21);
	_rand_z4 = ((_rand_z4 & 0xFFFFFF80u) << 13) ^ (((_rand_z4 <<  3) ^ _rand_z4) >> 12);
	return _rand_z1 ^ _rand_z2 ^ _rand_z3 ^ _rand_z4;
}

int rand(void)
{
	int val = (int)_rand_uint32();
	if(val < 0)
		val = ~val;
	
	val = val % RAND_MAX;
	return val;
}
