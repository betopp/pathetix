;tls.asm
;TLS getter for MuKe's libc on AMD64
;Bryan E. Topp <betopp@betopp.com> 2021

section .text
bits 64

align 16
global _tls ;_tls_t *_tls()
_tls:
	rdgsbase RAX
	ret
	