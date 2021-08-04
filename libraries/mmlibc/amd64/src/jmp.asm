;jmp.asm
;setjmp and longjmp on AMD64
;Bryan E. Topp <betopp@betopp.com> 2021

;These always happen as part of a function call.
;So, only registers preserved across function calls must be saved/loaded.

;Are threads allowed to exchange these buffers? Should we change GS base?

global _setjmp ;extern int _setjmp(jmp_buf env);
_setjmp:
	mov [RDI + (8 * 0)], RBX
	mov [RDI + (8 * 1)], RSP
	mov [RDI + (8 * 2)], RBP
	mov [RDI + (8 * 3)], R12
	mov [RDI + (8 * 4)], R13
	mov [RDI + (8 * 5)], R14
	mov [RDI + (8 * 6)], R15
	
	mov RAX, [RSP]
	mov [RDI + (8 * 7)], RAX
	
	mov RAX, 0
	ret
	
global _longjmp ;extern void _longjmp(jmp_buf env, int val);
_longjmp:
	mov RBX, [RDI + (8 * 0)]
	mov RSP, [RDI + (8 * 1)]
	mov RBP, [RDI + (8 * 2)]
	mov R12, [RDI + (8 * 3)]
	mov R13, [RDI + (8 * 4)]
	mov R14, [RDI + (8 * 5)]
	mov R15, [RDI + (8 * 6)]
	
	mov RAX, [RDI + (8 * 7)]
	mov [RSP], RAX
	
	mov RAX, RSI
	cmp RAX, 0
	jne .noinc
	inc RAX
	.noinc:
	ret


global sigsetjmp ;int sigsetjmp(sigjmp_buf env, int savemask)
sigsetjmp:
	mov [RDI], RSI ;store savemask value
	mov RAX, 0 ;default signals mask, and also the savemask value that avoids saving signals
	cmp RSI, RAX ;check if we want to save signals
	je .nosigs
		push RDI ;preserve buf
		mov RDI, 0 ;SIG_BLOCK
		mov RSI, 0 ;Block nothing, i.e. px_sigmask returns the mask
		extern px_sigmask
		call px_sigmask ;leaves signals mask in RAX
		pop RDI ;restore buf
	.nosigs:
	mov [RDI + 8], RAX ;store signals mask (or 0 if we didn't want to save it)
	
	add RDI, 16
	jmp _setjmp


global siglongjmp ;void siglongjmp(sigjmp_buf env, int val)
siglongjmp:
	mov RAX, [RDI] ;get old value of savemask
	cmp RAX, 0
	je .nosigs
		push RDI ;preserve env
		mov RSI, [RDI + 8] ;use previously saved signals
		mov RDI, 2 ;SIG_SETMASK
		call px_sigmask
		pop RDI
	.nosigs:
	
	;Do the rest of the jump
	add RDI, 16
	jmp _longjmp


global setjmp ;int setjmp(jmp_buf env)
setjmp:
	mov RSI, 1 ;setjmp is just sigsetjmp with savemask == true.
	jmp sigsetjmp

global longjmp ;void longjmp(jmp_buf env, int val)
longjmp:
	jmp siglongjmp

