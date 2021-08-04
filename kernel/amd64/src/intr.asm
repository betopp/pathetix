;intr.asm
;Interrupt management functions on AMD64
;Bryan E. Topp <betopp@betopp.com> 2021
section .text
bits 64

align 16
global hal_intr_ei ;bool hal_intr_ei(bool enable);
hal_intr_ei:
	;Check old flags for whether interrupts were enabled
	pushfq
	pop RAX
	and RAX, (1<<9)
	shr RAX, 9 ;Return value, Interrupt Flag as 0 or 1
	
	;You want the moustache on or off?
	cmp RDI, 0
	jnz .enable
		cli
		ret
	.enable:
		sti
		ret

align 16
global hal_intr_halt ;void hal_intr_halt(void);
hal_intr_halt:
	;On AMD64, "sti" enables interrupts after execution of the following instruction.
	;So a sequence of sti/hlt will atomically "halt with interrupts enabled".
	sti
	hlt
	ret
