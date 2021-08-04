//pt.c
//Pagetable management on AMD64
//Bryan E. Topp <betopp@betopp.com>

#include "hal_kspc.h"
#include "hal_uspc.h"
#include "hal_spl.h"
#include "pmem.h"
#include <stdint.h>

//Defined in cpuinit.asm
extern uint64_t cpuinit_pml4[];
extern uint64_t cpuinit_pdpt[];

//Defined in linker script - this is the earliest address for the kernel as loaded
extern uint8_t _KSPACE_BASE[];

//Spinlock protecting kernel-space
static hal_spl_t kspace_spl;

//Address mask to turn a pagetable entry into a frame address
#define ADDRMASK 0x0FFFFFFFFFFFF000

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

//Returns the Page Directory Base Register
static inline uint64_t getcr3(void)
{
	uint64_t addr;
	asm volatile ("mov %%cr3, %%rax": "=a"(addr) : );
	return addr;
}

//Sets a mapping in a page table, allocating frames as needed.
int pt_set(uint64_t pml4, uint64_t addr, uint64_t frame, uint64_t flags)
{
	uint64_t pml4_idx = (addr >> 39) % 512;
	uint64_t pml4_entry = pmem_read(pml4 + (8 * pml4_idx));
	if(!(pml4_entry & 1))
	{
		if(frame == 0)
			return 0; //Already not mapped
		
		//PML4 doesn't reference a PDPT here - allocate one and insert it.
		pml4_entry = hal_frame_alloc();
		if(pml4_entry == 0)
			return -1; //No room for paging structures
		
		pmem_clrframe(pml4_entry);
		
		pml4_entry |= flags;
		pmem_write(pml4 + (8 * pml4_idx), pml4_entry);
	}
	
	uint64_t pdpt = pml4_entry & ADDRMASK;
	uint64_t pdpt_idx = (addr >> 30) % 512;
	uint64_t pdpt_entry = pmem_read(pdpt + (8 * pdpt_idx));
	if(!(pdpt_entry & 1))
	{
		if(frame == 0)
			return 0; //Already not mapped
		
		//PDPT doesn't reference a PD here - allocate one and insert it.
		pdpt_entry = hal_frame_alloc();
		if(pdpt_entry == 0)
			return -1; //No room for paging structures
		
		pmem_clrframe(pdpt_entry);
		
		pdpt_entry |= flags;
		pmem_write(pdpt + (8 * pdpt_idx), pdpt_entry);
	}
	
	uint64_t pd = pdpt_entry & ADDRMASK;
	uint64_t pd_idx = (addr >> 21) % 512;
	uint64_t pd_entry = pmem_read(pd + (8 * pd_idx));
	if(!(pd_entry & 1))
	{
		if(frame == 0)
			return 0; //Already not mapped
		
		//PD doesn't reference a PT here - allocate one and insert it.
		pd_entry = hal_frame_alloc();
		if(pd_entry == 0)
			return -1; //No room for paging structures
		
		pmem_clrframe(pd_entry);
		
		pd_entry |= flags;
		pmem_write(pd + (8 * pd_idx), pd_entry);
	}
	
	uint64_t pt = pd_entry & ADDRMASK;
	uint64_t pt_idx = (addr >> 12) % 512;
	if(frame == 0)
		pmem_write(pt + (8 * pt_idx), 0);
	else
		pmem_write(pt + (8 * pt_idx), frame | flags);
	
	invlpg(addr);
	return 0; //Success
}

//Looks up a mapping in a page table
hal_frame_id_t pt_get(uint64_t pml4, uint64_t addr)
{
	uint64_t pml4_idx = (addr >> 39) % 512;
	uint64_t pml4_entry = pmem_read(pml4 + (8 * pml4_idx));
	if(!(pml4_entry & 1))
		return 0;
	
	uint64_t pdpt = pml4_entry & ADDRMASK;
	uint64_t pdpt_idx = (addr >> 30) % 512;
	uint64_t pdpt_entry = pmem_read(pdpt + (8 * pdpt_idx));
	if(!(pdpt_entry & 1))
		return 0;
	
	uint64_t pd = pdpt_entry & ADDRMASK;
	uint64_t pd_idx = (addr >> 21) % 512;
	uint64_t pd_entry = pmem_read(pd + (8 * pd_idx));
	if(!(pd_entry & 1))
		return 0;
	
	uint64_t pt = pd_entry & ADDRMASK;
	uint64_t pt_idx = (addr >> 12) % 512;
	uint64_t pt_entry = pmem_read(pt + (8 * pt_idx));
	if(!(pt_entry & 1))
		return 0;
	
	return pt_entry & ADDRMASK;
}


void hal_kspc_bound(uintptr_t *start_out, uintptr_t *end_out)
{
	//Dynamically allocate in the virtual region below the kernel as-linked.
	
	//Restrict to the upper 512GB of memory, so we never stray outside a single PDPT.
	//That way, all user-space PML4s can permanently include the kernel by referencing one PDPT at the end.
	
	//Restrict to outside the first/last PDs of the PDPT, so we don't mess with early mappings.
	
	*start_out = (4096ull*512*512) - (4096ull * 512 * 512 * 512);
	*end_out = ((uintptr_t)_KSPACE_BASE) - (4096ull*512*512);
}

int hal_kspc_set(uintptr_t vaddr, hal_frame_id_t frame)
{
	hal_spl_lock(&kspace_spl);
	int retval = pt_set((uint64_t)cpuinit_pml4 - (uintptr_t)_KSPACE_BASE, vaddr, frame, 3);
	hal_spl_unlock(&kspace_spl);
	return retval;
}

hal_frame_id_t hal_kspc_get(uintptr_t vaddr)
{
	hal_spl_lock(&kspace_spl);
	hal_frame_id_t retval = pt_get((uint64_t)cpuinit_pml4 - (uintptr_t)_KSPACE_BASE, vaddr);
	hal_spl_unlock(&kspace_spl);
	return retval;
}

hal_uspc_id_t hal_uspc_new(void)
{
	//Allocate a frame to hold the PML4
	hal_frame_id_t upml4 = hal_frame_alloc();
	if(upml4 == 0)
		return 0;
	
	//Clear the PML4 except for the top entry referring to the kernel's PDPT
	pmem_clrframe(upml4);
	extern uint64_t cpuinit_pdpt[];
	pmem_write(upml4 + (511 * 8), ((uint64_t)cpuinit_pdpt - (uintptr_t)_KSPACE_BASE) | 3);
	
	return upml4;
}

void hal_uspc_delete(hal_uspc_id_t id)
{
	//Work through all PDPTs referenced by the PML4 - except the last one, which refers to kernel-space
	uint64_t pml4 = id;
	for(int pml4e = 0; pml4e < 511; pml4e++)
	{
		uint64_t pml4_entry = pmem_read(pml4 + (8 * pml4e));
		if(!(pml4_entry & 1))
			continue; //No PDPT referenced here
		
		uint64_t pdpt = pml4_entry & ADDRMASK;
		
		//Work through all PDs referenced by the PDPT
		for(int pdpte = 0; pdpte < 512; pdpte++)
		{
			uint64_t pdpt_entry = pmem_read(pdpt + (8 * pdpte));
			if(!(pdpt_entry & 1))
				continue; //No PD referenced here
			
			uint64_t pd = pdpt_entry & ADDRMASK;
				
			//Work through all PTs referenced by the PD
			for(int pde = 0; pde < 512; pde++)
			{
				uint64_t pd_entry = pmem_read(pd + (8 * pde));
				if(!(pd_entry & 1))
					continue; //No PT referenced here
				
				uint64_t pt = pd_entry & ADDRMASK;
				
				//All actual data frames should have been freed already - the PT should be empty.
				hal_frame_free(pt);
			}
			
			//With all PTs freed, free the PD
			hal_frame_free(pd);
		}
		
		//With all PDs freed, free the PDPT
		hal_frame_free(pdpt);
	}
	
	//With all PDPTs freed, free the PML4
	hal_frame_free(pml4);
	return;
}

int hal_uspc_set(hal_uspc_id_t id, uintptr_t vaddr, hal_frame_id_t frame)
{
	return pt_set(id, vaddr, frame, 7);
}

hal_frame_id_t hal_uspc_get(hal_uspc_id_t id, uintptr_t vaddr)
{
	return pt_get(id, vaddr);
}

void hal_uspc_activate(hal_uspc_id_t id)
{
	if(id == HAL_USPC_ID_INVALID)
		setcr3((uintptr_t)cpuinit_pml4 - (uintptr_t)_KSPACE_BASE); //No-userspace case - just kernel pagetables
	else
		setcr3(id);
}

hal_uspc_id_t hal_uspc_current(void)
{
	return getcr3();
}

void hal_uspc_bound(uintptr_t *start_out, uintptr_t *end_out)
{
	*start_out = hal_frame_size(); //+1 page is first usable
	*end_out = 0x800000000000ul; //bottom 47-bit space usable as AMD64 canonical addresses
}
