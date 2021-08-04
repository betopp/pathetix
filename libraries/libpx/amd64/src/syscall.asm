;syscall.asm
;System call stubs for AMD64
;Bryan E. Topp <betopp@betopp.com> 2021

section .text
bits 64

;Define entry-points in assembly for each number of system call parameters.
;AMD64 SysVABI passes parameters in registers.
;But, AMD64 SYSCALL overwrites RCX.
;So set aside the RCX/4th parameter in R10, and then trigger a system-call.
align 16
global _pxcall0
global _pxcall1
global _pxcall2
global _pxcall3
global _pxcall4
global _pxcall5
_pxcall0:
_pxcall1:
_pxcall2:
_pxcall3:
_pxcall4:
_pxcall5:
	mov R10, RCX
	syscall
	ret
