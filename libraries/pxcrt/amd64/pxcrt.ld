/* pxcrt.ld
 * Linker script for userland on AMD64
 * Bryan E. Topp <betopp@betopp.com> 2021
 */

ENTRY(_pxcrt_entry)

SECTIONS {

	. = 0x10000000;

	.text ALIGN(0x1000) :
	{
		*(.header)
		*(.text)
		*(.note*)
		*(.init)
		*(.fini)
		. = ALIGN(0x1000);
	}
	
	. = . + 0x1000;
	
	.rodata ALIGN(0x1000) :
	{
		*(.rodata*)
		. = ALIGN(0x1000);
	}
	
	. = . + 0x1000;
	
	.data ALIGN(0x1000) :
	{
		*(.data)
		. = ALIGN(0x1000);
	}
	
	. = . + 0x1000;
	
	.bss ALIGN(0x1000) :
	{
		*(COMMON)
		*(.bss)
		. = ALIGN(0x1000);
	}

	. = . + 0x1000;
	. = ALIGN(0x1000);

	
}
