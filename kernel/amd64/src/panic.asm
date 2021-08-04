;panic.asm
;Panic handling on AMD64
;Bryan E. Topp <betopp@betopp.com> 2021

section .text
bits 64

global hal_panic ;void hal_panic(const char *str)
hal_panic:
	cli
	hlt
	jmp hal_panic
