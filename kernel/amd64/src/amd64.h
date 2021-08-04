//amd64.h
//C-language definitions for AMD64 architecture features
//Bryan E. Topp <betopp@betopp.com> 2021
#ifndef AMD64_H
#define AMD64_H

#include <stdint.h>

//Pagetable entry
typedef struct amd64_pae_pte_s
{
	uint64_t p        : 1;  //Present
	uint64_t rw       : 1;  //Writable
	uint64_t us       : 1;  //User-accessible
	uint64_t pwt      : 1;  //Page write-through
	uint64_t pcd      : 1;  //Page cache-disable
	uint64_t a        : 1;  //Accessed
	uint64_t d        : 1;  //Dirty
	uint64_t pat      : 1;  //Page attribute table
	uint64_t g        : 1;  //Global
	uint64_t avl      : 3;  //Available for OS (unused in MMK)
	uint64_t frameidx : 40; //Physical frame address 51:12
	uint64_t avl2     : 7;  //Available for OS (unused in MMK)
	uint64_t mpk      : 4;  //Memory protection key
	uint64_t nx       : 1;  //No Execute
	
} amd64_pae_pte_t;

//Page directory entry
typedef struct amd64_pae_pde_s
{
	uint64_t p        : 1;  //Present
	uint64_t rw       : 1;  //Writable
	uint64_t us       : 1;  //User-accessible
	uint64_t pwt      : 1;  //Page write-through
	uint64_t pcd      : 1;  //Page cache-disable
	uint64_t a        : 1;  //Accessed
	uint64_t ign0     : 1;  //Ignored
	uint64_t mbz      : 1;  //Must be zero (indicates 2MByte pages otherwise)
	uint64_t ign1     : 1;  //Ignored
	uint64_t avl      : 3;  //Available for OS (unused in MMK)
	uint64_t frameidx : 40; //Physical frame address 51:12
	uint64_t avl2     : 11; //Available for OS (unused in MMK)
	uint64_t nx       : 1;  //No Execute	
	
} amd64_pae_pde_t;

//Page directory pointer entry
typedef struct amd64_pae_pdpe_s
{
	uint64_t p        : 1;  //Present
	uint64_t rw       : 1;  //Writable
	uint64_t us       : 1;  //User-accessible
	uint64_t pwt      : 1;  //Page write-through
	uint64_t pcd      : 1;  //Page cache-disable
	uint64_t a        : 1;  //Accessed
	uint64_t ign0     : 1;  //Ignored
	uint64_t mbz      : 1;  //Must be zero (indicates 1GByte pages otherwise)
	uint64_t ign1     : 1;  //Ignored
	uint64_t avl      : 3;  //Available for OS (unused in MMK)
	uint64_t frameidx : 40; //Physical frame address 51:12
	uint64_t avl2     : 11; //Available for OS (unused in MMK)
	uint64_t nx       : 1;  //No Execute
	
} amd64_pae_pdpe_t;

//Page map level 4 entry
typedef struct amd64_pae_pml4e_s
{
	uint64_t p        : 1;  //Present
	uint64_t rw       : 1;  //Writable
	uint64_t us       : 1;  //User-accessible
	uint64_t pwt      : 1;  //Page write-through
	uint64_t pcd      : 1;  //Page cache-disable
	uint64_t a        : 1;  //Accessed
	uint64_t ign      : 1;  //Ignored
	uint64_t mbz      : 2;  //Must be zero
	uint64_t avl      : 3;  //Available for OS (unused in MMK)
	uint64_t frameidx : 40; //Physical frame address 51:12
	uint64_t avl2     : 11; //Available for OS (unused in MMK)
	uint64_t nx       : 1;  //No Execute
	
} amd64_pae_pml4e_t;

//Invalidates the TLB entry for the given page address
static inline void invlpg(uint64_t addr)
{
	asm volatile ("invlpg (%%rax)": :"a" (addr));
}

//Sets the Page Directory Base Register
static inline void setcr3(uint64_t addr)
{
	asm volatile ("mov %%rax, %%cr3": :"a" (addr));
}

//Sets the Page Directory Base Register
static inline uint64_t getcr3(void)
{
	uint64_t addr;
	asm volatile ("mov %%cr3, %%rax": "=a"(addr) : );
	return addr;
}

//Writes a byte to the given IO port.
static inline void outb(uint16_t port, uint8_t byte)
{
	asm volatile ("out %%al,%%dx": :"d" (port), "a" (byte));
}

//Writes a word to the given IO port.
static inline void outw(uint16_t port, uint16_t word)
{
	asm volatile ("out %%ax,%%dx": :"d" (port), "a" (word));
}

//Writes a dword to the given IO port.
static inline void outd(uint16_t port, uint32_t dword)
{
	asm volatile ("out %%eax,%%dx": :"d" (port), "a" (dword));
}

//Reads a byte from the given IO port.
static inline uint8_t inb(uint16_t port)
{
	uint8_t byte;
	asm volatile ("in %%dx,%%al":"=a"(byte):"d"(port));
	return byte;
}

//Reads a word from the given IO port.
static inline uint16_t inw(uint16_t port)
{
	uint16_t word;
	asm volatile ("in %%dx,%%ax":"=a"(word):"d"(port));
	return word;
}

//Reads a dword from the given IO port.
static inline uint32_t ind(uint16_t port)
{
	uint32_t dword;
	asm volatile ("in %%dx,%%eax":"=a"(dword):"d"(port));
	return dword;
}

#endif //AMD64_H
