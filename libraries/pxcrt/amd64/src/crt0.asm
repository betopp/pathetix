;crt0.asm
;Entry point for C runtime
;Bryan E. Topp <betopp@betopp.com> 2021

bits 64
section .text
	
;Entry point, for both initial startup and for signal handling
align 16
global _pxcrt_entry
_pxcrt_entry:
	jmp _pxcrt_entry_startup ;Symbol + 0 = initial entry
	align 16
	jmp _pxcrt_entry_signal ;Symbol + 16 = entry on signal


;Entry for startup
_pxcrt_entry_startup:

	;Set thread-local storage pointer for initial thread
	mov RAX, _pxcrt_tls0
	wrgsbase RAX

	;Zero GPRs
	mov RAX, 0
	mov RCX, RAX
	mov RDX, RAX
	mov RBX, RAX
	mov RSI, RAX
	mov RDI, RAX
	mov RSP, RAX
	mov RBP, RAX
	mov R8, RAX
	mov R9, RAX
	mov R10, RAX
	mov R11, RAX
	mov R12, RAX
	mov R13, RAX
	mov R14, RAX
	mov R15, RAX
	
	;Use static stack
	mov RSP, _pxcrt_stack.top
	
	;Call libc which will call main and then exit
	extern _libc_entry
	call _libc_entry
	
	hlt
	jmp 0
	jmp _pxcrt_entry
	
	
;Entry for signal handler
_pxcrt_entry_signal:

	;Avoid clobbering red-zone of old stack, in case this signal was unexpected.
	sub RSP, 128

	;Handle signal
	extern _libc_signalled
	call _libc_signalled
	
	;libc_signalled should ask the kernel to return to the signalled context
	hlt
	jmp 0
	jmp _pxcrt_entry
	
bits 64
section .bss
	
;Space for stack
alignb 4096
_pxcrt_stack:
	resb 4096 * 4
	.top:	

	
bits 64
section .data
	
;Space for initial thread TLS
align 4096
_pxcrt_tls0:
	dq _pxcrt_tls0 ;TLS starts with pointer to itself
	times (4096 - 8) db 0
	.top:
	
