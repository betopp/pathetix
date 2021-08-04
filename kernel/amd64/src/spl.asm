;spl.asm
;Spinlock implementation for AMD64
;Bryan E. Topp <betopp@betopp.com> 2021
section .text
bits 64

align 16
global hal_spl_lock ;void hal_spl_lock(hal_spl_t *spl);
hal_spl_lock:
	;Lock attempt - try to actually get the spinlock, using a locking atomic.
	mov AX, 0x0100         ;AH = 1, AL = 0
	lock cmpxchg [RDI], AH ;Compare lock value with 0, and if it was, overwrite with 1. Otherwise, 
	cmp AL, 0              ;If lock value wasn't 0, AL is overwritten with lock value
	jne .waitzero          ;If we failed to get the lock, stop with the locked atomics for a moment
	
	;Got the lock
	mfence                 ;Make sure no subsequent memory operations can happen until we hold the lock
	ret
	
	;If we failed to get the lock, wait until the memory shows 0, using non-locked accesses.
	;Vahalia claims that this improves performance when a lock is contested.
	.waitzero:
	pause ;Architectural hint - tell the CPU we're in a busy loop
	cmp [RDI], byte 0 ;Do a normal read to see if the spinlock has been released
	jne .waitzero ;Repeat until we see it at least momentarily zeroed
	jmp hal_spl_lock ;Try again for real

align 16
global hal_spl_try ;bool hal_spl_try(hal_spl_t *spl);
hal_spl_try:
	;Lock attempt - try to actually get the spinlock, using a locking atomic.
	mov AX, 0x0100         ;AH = 1, AL = 0
	lock cmpxchg [RDI], AH ;Compare lock value with 0, and if it was, overwrite with 1. Otherwise, 
	cmp AL, 0              ;If lock value wasn't 0, AL is overwritten with lock value
	jne .failed            ;If we failed to get the lock, just return
	
	;Got the lock
	mfence                 ;Make sure no subsequent memory operations can happen until we hold the lock
	mov RAX, 1
	ret
	
	;Didn't get the lock
	.failed:
	mov RAX, 0
	ret


align 16
global hal_spl_unlock ;void hal_spl_unlock(hal_spl_t *spl);
hal_spl_unlock:
	mfence                ;Make sure all memory operations are completed before releasing the lock
	mov byte [RDI], 0     ;Zero the lock value
	ret

