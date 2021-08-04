;crti.asm
;Function prologues for init/fini functions in C runtime
;Bryan E. Topp <betopp@betopp.com> 2021

bits 64
section .init
	
;Linked at the beginning of the .init section
push RBP
mov RBP, RSP


bits 64
section .fini

;Linked at the beginning of the .fini section
push RBP
mov RBP, RSP

