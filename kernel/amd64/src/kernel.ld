/* kernel.ld
 * Linker script for kernel on AMD64
 * Bryan E. Topp <betopp@betopp.com> 2021
 */

ENTRY(multiboot_entry)

SECTIONS {

	/*
	Put the kernel at the last PDPT entry of the last PML4 entry (-1GB)
	*/
	_KSPACE_BASE = 0xFFFFFFFFC0000000;
	
	/*
	We treat _KSPACE_BASE as the difference between virtual and physical addresses for the kernel as-linked.
	Physically load the kernel at +1MByte to avoid BIOS crap.
	*/
	. = 0x100000 + _KSPACE_BASE;

	_MULTIBOOT_LOAD_START = . - _KSPACE_BASE; /* Physical start of loading */
	_KERNEL_START = .;
	.text ALIGN(0x1000) : AT(ADDR(.text) - _KSPACE_BASE)
	{
	
		*(.boot) /*Put multiboot at beginning, and use physical space */
		
		. = ALIGN(0x1000);
		*(.text)
		
		*(.rodata*)
		*(.note*)
	}
	
	.data ALIGN(0x1000) : AT(ADDR(.data) - _KSPACE_BASE)
	{
		. = ALIGN(0x1000);
		*(.data)
	}
	
	_MULTIBOOT_LOAD_END = . - _KSPACE_BASE; /* Physical end of data in file */

	.bss ALIGN(0x1000) : AT(ADDR(.bss) - _KSPACE_BASE)
	{
		*(COMMON)
		
		. = ALIGN(0x1000);
		*(.bss)
	}
	
	/*Pad end-of-kernel to page boundary*/
	.endpad ALIGN(0x1000) : AT(ADDR(.endpad) - _KSPACE_BASE)
	{
		_ENDPAD_MARKER = .;
	}

	_MULTIBOOT_ZERO_END = . - _KSPACE_BASE; /* Physical end of kernel as linked */
	_KERNEL_END = .;
}
