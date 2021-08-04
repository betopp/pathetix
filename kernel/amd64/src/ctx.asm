;ctx.asm
;Context saving/loading on AMD64
;Bryan E. Topp <betopp@betopp.com> 2021
bits 64
section .text

;Structure of CPU context as saved
;The context saving/loading functions are only run from a proper SysVABI function call.
;So we only need to save registers that are preserved across function calls.
%define CTX_OFFS_RBX (8 * 0)
%define CTX_OFFS_RSP (8 * 1)
%define CTX_OFFS_RBP (8 * 2)
%define CTX_OFFS_R12 (8 * 3)
%define CTX_OFFS_R13 (8 * 4)
%define CTX_OFFS_R14 (8 * 5)
%define CTX_OFFS_R15 (8 * 6)
%define CTX_OFFS_KGS (8 * 7) ;Kernel GS base
%define CTX_OFFS_UGS (8 * 8) ;User GS base
%define CTX_OFFS_PDB (8 * 9) ;Page directory base register
%define CTX_SIZE     (8 * 10)


align 16
global hal_ctx_size ;size_t hal_ctx_size(void);
hal_ctx_size:
	mov RAX, CTX_SIZE
	ret

align 16
global hal_ctx_reset ;void hal_ctx_reset(void *dst, void (*entry)(void), void *stack_top, void *ktls);
hal_ctx_reset:

	;Store kernel GS base (kernel TLS)
	mov [RDI + CTX_OFFS_KGS], RCX
	
	;Store kernel page-directory base register
	mov RAX, CR3
	mov [RDI + CTX_OFFS_PDB], RAX
	
	;Push the "entry" onto the given stack, so we return there when switching to the context
	sub RDX, 8
	mov [RDX], RSI
	
	;Store that as the stack-pointer
	mov [RDI + CTX_OFFS_RSP], RDX
	
	;Zero all the rest of the structure
	mov RAX, 0
	mov [RDI + CTX_OFFS_RBX], RAX
	mov [RDI + CTX_OFFS_RBP], RAX
	mov [RDI + CTX_OFFS_R12], RAX
	mov [RDI + CTX_OFFS_R13], RAX
	mov [RDI + CTX_OFFS_R14], RAX
	mov [RDI + CTX_OFFS_R15], RAX
	
	ret

align 16
global hal_ctx_switch ;void hal_ctx_switch(void *save, const void *load);
hal_ctx_switch:

	;Disable interrupts while switching contexts - so we don't get stuck with the wrong GS-base
	cli

	;Save caller-owned general-purpose registers
	mov [RDI + CTX_OFFS_RBX], RBX
	mov [RDI + CTX_OFFS_RSP], RSP
	mov [RDI + CTX_OFFS_RBP], RBP
	mov [RDI + CTX_OFFS_R12], R12
	mov [RDI + CTX_OFFS_R13], R13
	mov [RDI + CTX_OFFS_R14], R14
	mov [RDI + CTX_OFFS_R15], R15
	
	;Save user GS base
	swapgs
	rdgsbase RAX
	mov [RDI + CTX_OFFS_UGS], RAX
	
	;Save kernel GS base
	swapgs
	rdgsbase RAX
	mov [RDI + CTX_OFFS_KGS], RAX
	
	;Store page directory base register
	mov RAX, CR3
	mov [RDI + CTX_OFFS_PDB], RAX
	
	;Load caller-owned general-purpose registers
	mov RBX, [RSI + CTX_OFFS_RBX]
	mov RSP, [RSI + CTX_OFFS_RSP]
	mov RBP, [RSI + CTX_OFFS_RBP]
	mov R12, [RSI + CTX_OFFS_R12]
	mov R13, [RSI + CTX_OFFS_R13]
	mov R14, [RSI + CTX_OFFS_R14]
	mov R15, [RSI + CTX_OFFS_R15]
	
	;Load user GS base
	mov RAX, [RSI + CTX_OFFS_UGS]
	wrgsbase RAX
	swapgs
	
	;Load kernel GS base
	mov RAX, [RSI + CTX_OFFS_KGS]
	wrgsbase RAX
	
	;Load page directory base register
	mov RAX, [RSI + CTX_OFFS_PDB]
	mov CR3, RAX
	
	
	ret
