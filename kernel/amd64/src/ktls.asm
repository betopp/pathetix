;ktls.asm
;Kernel thread pointer on AMD64
;Bryan E. Topp <betopp@betopp.com> 2021

section .text
bits 64

;Fun fact - we store the thread pointer in the GS Base register but never actually access through GS.
;rdgsbase/wrgsbase allow us to just treat GS Base like another register.

align 16
global hal_ktls_set ;void hal_ktls_set(void *ptr);
hal_ktls_set:
	wrgsbase RDI
	ret

align 16
global hal_ktls_get ;void *hal_ktls_get(void);
hal_ktls_get:
	rdgsbase RAX
	ret
