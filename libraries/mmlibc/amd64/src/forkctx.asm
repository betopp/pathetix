;forkctx.asm
;Context saving/loading across fork on AMD64
;Bryan E. Topp <betopp@betopp.com> 2021

section .text
bits 64

;setjmp-style call used in forking
global _forkctx_save ;int _forkctx_save();
_forkctx_save:
	mov [_forkctx_buf + (8 * 0)], RBX
	mov [_forkctx_buf + (8 * 1)], RSP
	mov [_forkctx_buf + (8 * 2)], RBP
	mov [_forkctx_buf + (8 * 3)], R12
	mov [_forkctx_buf + (8 * 4)], R13
	mov [_forkctx_buf + (8 * 5)], R14
	mov [_forkctx_buf + (8 * 6)], R15
	
	mov RAX, [RSP]
	mov [_forkctx_buf + (8 * 7)], RAX
	
	rdgsbase RAX
	mov [_forkctx_buf + (8 * 8)], RAX
	
	mov RAX, 0 ;First return - 0
	ret

;longjmp-style call used in forking - Entry point after fork
global _forkctx_load ;void _forkctx_load();
_forkctx_load:
	mov RBX, [_forkctx_buf + (8 * 0)]
	mov RSP, [_forkctx_buf + (8 * 1)]
	mov RBP, [_forkctx_buf + (8 * 2)]
	mov R12, [_forkctx_buf + (8 * 3)]
	mov R13, [_forkctx_buf + (8 * 4)]
	mov R14, [_forkctx_buf + (8 * 5)]
	mov R15, [_forkctx_buf + (8 * 6)]
	
	mov RAX, [_forkctx_buf + (8 * 7)]
	mov [RSP], RAX
	
	mov RAX, [_forkctx_buf + (8 * 8)]
	wrgsbase RAX
	
	mov RAX, 1 ;Second return at our old stack pointer - 1
	ret	

section .bss
bits 64

;Space for register state preserved across fork
_forkctx_buf:
	resb 8 * 16
	
