;pmem.asm
;Physical memory access
;Bryan E. Topp <betopp@betopp.com> 2021

section .text
bits 64

extern hal_spl_lock
extern hal_spl_unlock

pmem_map:
	cmp RDI, [pmem_last]
	je .done

	;Page number of window
	mov RAX, pmem_window
	shr RAX, 12
	and RAX, 0x1FF 

	;PTE for frame we want to write to
	mov RCX, RDI
	and RCX, 0xFFFFFFFFFFFFF000
	or RCX, 3 
	
	extern cpuinit_pt
	mov [cpuinit_pt + (RAX * 8)], RCX
	invlpg [pmem_window]
	
	mov [pmem_last], RDI
	
	.done:
	ret

global pmem_read ;uint64_t pmem_read(uint64_t paddr);
pmem_read:

	;Lock spinlock
	push RDI
	mov RDI, pmem_spl
	call hal_spl_lock
	pop RDI

	;Remap the "window" to point at the page we care about
	call pmem_map
	
	;Read from it
	mov RDX, RDI
	and RDX, 0xFFF
	mov RAX, [pmem_window + RDX]
	
	;Unlock spinlock
	push RAX
	mov RDI, pmem_spl
	call hal_spl_unlock
	pop RAX
	
	ret

global pmem_write ;void pmem_write(uint64_t paddr, uint64_t data);
pmem_write:

	;Lock spinlock
	push RDI
	push RSI
	mov RDI, pmem_spl
	call hal_spl_lock
	pop RSI
	pop RDI
	
	;Remap the "window" to point at the page we care about
	call pmem_map
	
	;Write to it
	mov RDX, RDI
	and RDX, 0xFFF
	mov [pmem_window + RDX], RSI
	
	;Unlock spinlock
	mov RDI, pmem_spl
	call hal_spl_unlock
	
	ret
	
global pmem_clrframe ;void pmem_clrframe(uint64_t paddr);
pmem_clrframe:

	;Lock spinlock
	push RDI
	mov RDI, pmem_spl
	call hal_spl_lock
	pop RDI

	call pmem_map
	
	mov RDI, pmem_window
	mov RAX, 0
	mov RCX, 4096 / 8
	rep stosq
	
	;Unlock spinlock
	mov RDI, pmem_spl
	call hal_spl_unlock
	
	ret

global hal_frame_copy ;void hal_frame_copy(hal_frame_id_t dst, hal_frame_id_t src);
hal_frame_copy:
	push RSI
	push RDI
	mov RDI, pmem_spl
	call hal_spl_lock
	pop RDI
	pop RSI

	mov R11, RDI
	mov RDI, RSI
	call pmem_map
	
	mov RDI, pmem_temp
	mov RSI, pmem_window
	mov RCX, 4096 / 8
	rep movsq
	
	mov RDI, R11
	call pmem_map
	
	mov RDI, pmem_window
	mov RSI, pmem_temp
	mov RCX, 4096 / 8
	rep movsq
	
	mov RDI, pmem_spl
	call hal_spl_unlock
	
	ret	


section .bss
alignb 4096
pmem_window:
	resb 4096
	
alignb 4096
pmem_temp:
	resb 4096
	
alignb 8
pmem_spl:
	resb 8
	
alignb 8
pmem_last:
	resb 8
