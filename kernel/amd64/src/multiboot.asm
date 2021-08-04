;multiboot.asm
;Entry from bootloader
;Bryan E. Topp <betopp@betopp.com> 2021

section .boot ;Placed first by the linker
bits 32

;Amount of memory for saving multiboot loader's memory map
%define MULTIBOOT_MMAP_MAX 1024

;Amount of memory for saving multiboot loader's module list
%define MULTIBOOT_MODINFO_MAX 1024

;Magic number identifying multiboot header
%define MULTIBOOT_MAGIC 0x1BADB002

;Flags for multiboot loader
;Bit 16 set = we have address info embedded
;Bit 3 set = give us module information
;Bit 1 set = give us memory information
;Bit 0 set = align on 4KB boundaries
%define MULTIBOOT_FLAGS 0x0001000B

;Checksum for magic and flags
%define MULTIBOOT_CHECKSUM -(MULTIBOOT_MAGIC+MULTIBOOT_FLAGS)

;Physical location of kernel defined in linker script
extern _MULTIBOOT_LOAD_START
extern _MULTIBOOT_LOAD_END
extern _MULTIBOOT_ZERO_END

;Header that bootloader looks for
multiboot_header:
	dd MULTIBOOT_MAGIC ;Magic number
	dd MULTIBOOT_FLAGS ;Flags for bootloader
	dd MULTIBOOT_CHECKSUM ;Checksum of magic + flags
	dd _MULTIBOOT_LOAD_START + (multiboot_header - $$) ;Where the header is supposed to end up in memory
	dd _MULTIBOOT_LOAD_START ;First address to be loaded or zeroed in memory
	dd _MULTIBOOT_LOAD_END ;Last address to be loaded in memory
	dd _MULTIBOOT_ZERO_END ;Last address to be zeroed in memory
	dd _MULTIBOOT_LOAD_START + (multiboot_entry - $$) ;Entry point after kernel is loaded

;First code run after bootloader loads kernel
global multiboot_entry
multiboot_entry:
	
	;Check that we're actually loaded by multiboot
	cmp EAX, 0x2BADB002
	jne multiboot_fail
	
	;Get rid of EGA cursor
	mov AL, 0x0A
	mov DX, 0x3D4
	out DX, AL
	mov AL, 0x20
	inc DX
	out DX, AL
	
	;We should be in 32-bit protected mode without paging right now.
	;We'll need to turn symbols into physical addresses to use them.
	;Define a macro to help with this.
	%define PHYSADDR(symb) (symb - 0xFFFFFFFFC0000000)
	
	;Make sure our macro lines up with the linker script
	extern _KSPACE_BASE
	mov EAX, PHYSADDR(_KSPACE_BASE)
	cmp EAX, 0
	jne multiboot_fail
	
	;Look up the flags from the multiboot information structure.
	mov EAX, [EBX + 0]
	
	;Make sure multiboot gave us memory and module information
	and EAX, (1<<6) | (1<<3)
	cmp EAX, (1<<6) | (1<<3)
	jne multiboot_fail
	
	;Get memory-map location and length
	mov ESI, [EBX + 48]
	mov ECX, [EBX + 44]
	
	;Cap length to the amount of space we reserve
	cmp ECX, 0
	je .mmap_done
	cmp ECX, MULTIBOOT_MMAP_MAX
	jb .mmap_fits
		mov ECX, MULTIBOOT_MMAP_MAX
	.mmap_fits:
	
	mov [PHYSADDR(multiboot_mmap_size)], ECX
	
	;Copy the memory-map to our own storage for it
	mov EDI, PHYSADDR(multiboot_mmap_storage)
	rep movsb
	
	.mmap_done:
	
	;Get modules location and count
	mov ESI, [EBX + 24]
	mov ECX, [EBX + 20]
	shl ECX, 4 ;Each entry is 16 bytes
	
	;Cap length of modules to the amount of space we reserve
	cmp ECX, 0
	je .modinfo_done
	cmp ECX, MULTIBOOT_MODINFO_MAX
	jb .modinfo_fits
		mov ECX, MULTIBOOT_MODINFO_MAX
	.modinfo_fits:
	
	mov [PHYSADDR(multiboot_modinfo_size)], ECX
	
	;Copy the module info to our own storage for it
	mov EDI, PHYSADDR(multiboot_modinfo_storage)
	rep movsb
	
	.modinfo_done:
	
	;Mask all 8259 PIC interrupts before starting the kernel - we don't use the 8259.
	mov AL, 0xFF
	out 0x21, AL
	out 0xA1, AL
	
	;Set up AMD64 CPU and launch kernel
	extern cpuinit_entry
	clc
	jnc cpuinit_entry

multiboot_fail:
	.spin:
	jmp .spin


bits 64
section .text
	
align 16
global hal_bootfile_count ;int hal_bootfile_count(void);
hal_bootfile_count:
	mov RAX, [multiboot_modinfo_size]
	shr RAX, 4 ;Each module info from multiboot is 16 bytes
	ret

align 16
global hal_bootfile_addr ;hal_frame_id_t hal_bootfile_addr(int idx);
hal_bootfile_addr:
	;See if the module is in range
	mov RAX, [multiboot_modinfo_size]
	shr RAX, 4 ;Each module info from multiboot is 16 bytes
	cmp RDI, RAX ;First parameter in RDI
	jb .valid_id
		;Not a valid ID
		mov RAX, 0
		ret
	.valid_id:
	
	;Load the start/end of the module
	mov RAX, RDI
	shl RAX, 4
	mov ECX, [multiboot_modinfo_storage + RAX] ;Start of module
	mov EDX, [multiboot_modinfo_storage + RAX + 4] ;End of module
	
	;Make sure it's of nonzero size
	cmp EDX, ECX
	ja .valid_size
		;Module is zero-sized
		mov RAX, 0
		ret
	.valid_size:
	
	;Return first frame of module
	mov EAX, ECX
	ret

align 16
global hal_bootfile_size ;size_t hal_bootfile_size(int mod);
hal_bootfile_size:
	;See if the module is in range
	mov RAX, [multiboot_modinfo_size]
	shr RAX, 4 ;Each module info from multiboot is 16 bytes
	cmp RDI, RAX ;First parameter in RDI
	jb .valid_id
		;Not a valid ID
		mov RAX, 0
		ret
	.valid_id:
	
	;Load the start/end of the module
	mov RAX, RDI
	shl RAX, 4
	mov ECX, [multiboot_modinfo_storage + RAX] ;Start of module
	mov EDX, [multiboot_modinfo_storage + RAX + 4] ;End of module
	
	;Make sure it's of nonzero size
	cmp EDX, ECX
	ja .valid_size
		;Module is zero-sized
		mov RAX, 0
		ret
	.valid_size:
	
	;Return size of module
	mov EAX, EDX
	sub EAX, ECX
	ret


section .bss
bits 32

;Copy of multiboot's memory map info
alignb 16
global multiboot_mmap_storage
multiboot_mmap_storage:
	resb MULTIBOOT_MMAP_MAX

;Size of memory map info copied
alignb 16
global multiboot_mmap_size
multiboot_mmap_size:
	resb 8
	
;Copy of multiboot's module info
alignb 16
global multiboot_modinfo_storage
multiboot_modinfo_storage:
	resb MULTIBOOT_MODINFO_MAX
	
;Size of module info copied
alignb 16
global multiboot_modinfo_size
multiboot_modinfo_size:
	resb 8

