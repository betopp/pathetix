;crtn.asm
;Function epilogues for init/fini functions in C runtime
;Bryan E. Topp <betopp@betopp.com> 2021

bits 64
section .init
	
;Linked at the end of the .init section
pop RBP
ret


bits 64
section .fini

;Linked at the end of the .fini section
pop RBP
ret

