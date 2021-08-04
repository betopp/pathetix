;atomic.asm
;Atomic counters for amd64
;Bryan E. Topp <betopp@betopp.com> 2021

section .text
bits 64

global hal_atomic_inc ;uint64_t hal_atomic_inc(hal_atomic_t *atom);
hal_atomic_inc:
	mov EAX, [RDI] ;Get old value
	mov ECX, EAX
	inc ECX ;Make incremented value aside from old value
	lock cmpxchg [RDI], ECX ;Compares value in memory with EAX (old value), if eq, replaces with ECX (incremented)
	jnz hal_atomic_inc ;Try again if somebody else got there first
	mov EAX, ECX ;Return the value written
	ret

global hal_atomic_dec ;uint64_t hal_atomic_dec(hal_atomic_t *atom);
hal_atomic_dec:
	mov EAX, [RDI]
	mov ECX, EAX
	dec ECX ;Only difference from hal_atomic_inc
	lock cmpxchg [RDI], ECX
	jnz hal_atomic_dec
	mov EAX, ECX
	ret
