;cpuinit.asm
;AMD64 CPU setup
;Bryan E. Topp <betopp@betopp.com> 2021

;How many CPUs we support
%define CPU_MAX 256

;Where the trampoline for SMP initialization gets copied
%define SMP_STUB_ADDR 4096

extern hal_panic

section .text
bits 32

global cpuinit_entry
cpuinit_entry:

	;We should be in 32-bit protected mode without paging right now.
	;We'll need to turn symbols into physical addresses to use them.
	;Define a macro to help with this.
	%define PHYSADDR(symb) (symb - 0xFFFFFFFFC0000000)
	
	;Make sure our macro lines up with the linker script
	extern _KSPACE_BASE
	mov EAX, PHYSADDR(_KSPACE_BASE)
	cmp EAX, 0
	jne cpuinit_fail
	
	;Make sure the kernel as-linked fits in a single identity-mapped page table (2MBytes of space).
	mov ECX, 4096 * 511 ;We need to save one page at the top of the table for mapping the Local APIC.
	mov EDX, 4096 * 2 ;We need to save two pages at the bottom for identity-mapping a spot to start-up secondary cores.
	
	extern _KERNEL_START ;From linker script
	mov EAX, PHYSADDR(_KERNEL_START)
	cmp EAX, ECX
	jae cpuinit_fail
	cmp EAX, EDX
	jb cpuinit_fail
	
	extern _KERNEL_END ;From linker script
	mov EAX, PHYSADDR(_KERNEL_END)
	cmp EAX, ECX
	jae cpuinit_fail
	cmp EAX, EDX
	jb cpuinit_fail
	
	;Verify that the CPU supports the features that we need for this kernel.
	;Todo - do something reasonable if we see the wrong CPU type. For now, just crash.
	
	;Check that we have CPUID support.
	mov ESP, PHYSADDR(cpuinit_initstack.top)
	pushfd ;Save EFLAGS
	pop EAX ;Store EFLAGS in EAX
	mov EBX, EAX ;Save in EBX for later testing
	xor EAX, (1<<21) ;Toggle bit 21
	push EAX ;Push modified flags to stack
	popfd ;Save changed flags to EFLAGS
	pushfd ;Push EFLAGS back onto stack
	pop EAX ;Get value in EAX
	cmp EAX, EBX ;See if we were able to successfully change the bit
	jz cpuinit_fail ;Jumps in failure case
	
	;Use CPUID to check that we have Long Mode support
	mov EAX, 0x80000001
	cpuid
	and EDX, 1<<29 ;Test Long Mode bit
	jz cpuinit_fail ;Jumps in failure case
	
	;Use CPUID to check that we have APIC support
	mov EAX, 0x00000001
	cpuid
	and EDX, 1<<9 ;Test APIC bit
	jz cpuinit_fail ;Jumps in failure case
	
	;Use CPUID to check that we have RDMSR support
	mov EAX, 0x00000001
	cpuid
	and EDX, 1<<5 ;Test MSR bit
	jz cpuinit_fail ;Jumps in failure case
	
	;Use CPUID to check that we have [RD/WR][FS/GS]BASE
	mov EAX, 0x00000007
	mov ECX, 0
	cpuid
	and EBX, 1<<0 ;Test FSGSBASE bit
	jz cpuinit_fail ;Jumps in failure case
	
	;Use CPUID to check that we support SYSCALL/SYSRET
	mov EAX, 0x80000001
	cpuid
	and EDX, 1<<11;Test SysCallSysRet bit
	jz cpuinit_fail ;Jumps in failure case
	
	
	;Using physical addresses, set up the kernel paging structures.
	;Map both identity (as loaded at 1MByte) and virtual spaces (-1GByte+1MByte).
	
	;Point PML4[0] and PML4[511] at PDPT
	mov EAX, PHYSADDR(cpuinit_pdpt)
	or EAX, 3 ;Present, writable
	mov [PHYSADDR(cpuinit_pml4) + (8 * 511)], EAX
	mov [PHYSADDR(cpuinit_pml4)], EAX
	
	;Point PDPT[0] and PDPT[511] at PD
	mov EAX, PHYSADDR(cpuinit_pd)
	or EAX, 3 ;Present, writable
	mov [PHYSADDR(cpuinit_pdpt) + (8*511)], EAX
	mov [PHYSADDR(cpuinit_pdpt)], EAX
	
	;Point PD[0] at PT
	mov EAX, PHYSADDR(cpuinit_pt)
	or EAX, 3 ;Present, writable
	mov [PHYSADDR(cpuinit_pd)], EAX
	
	;Fill PT entries to cover kernel
	mov EAX, PHYSADDR(_KERNEL_START)
	shr EAX, 12
	mov EBX, PHYSADDR(_KERNEL_END)
	shr EBX, 12
	mov EDI, PHYSADDR(cpuinit_pt)
	.pt_loop:
		cmp EAX, EBX
		jae .pt_done
		
		mov ECX, EAX
		shl ECX, 12 ;Turn back into address
		or ECX, 3 ;Present, writable
		mov [EDI + (8 * EAX)], ECX
		
		inc EAX
		jmp .pt_loop
	.pt_done:
	
	;Point the CPU at the PML4 now that it's ready
	mov EAX, PHYSADDR(cpuinit_pml4)
	mov CR3, EAX
	
	;Turn on Physical Address Extension and Long Mode Enable to indicate 4-level paging for Long Mode
	mov EAX, CR4
	or EAX, (1<<5) ;PAE
	mov CR4, EAX
	
	mov ECX, 0xC0000080 ;EFER
	rdmsr
	or EAX, (1<<8) ;LME
	wrmsr
	
	;Enable paging, still 32-bit code, identity-mapped space for now. This should activate Long Mode, because LME was set.
	mov EAX, CR0
	or EAX, (1<<31) ;PG
	mov CR0, EAX
	
	;Now that we're in Long Mode we can jump to 64-bit code.
	;Do so, using a temporary global descriptor table + 64-bit code descriptor.
	;(We still can't refer to 64-bit addresses yet.)
	mov EDI, PHYSADDR(.gdtr)
	lgdt [EDI]
	jmp 8 : PHYSADDR(.target64)
	.gdt:
		dq 0 ;Null
		db 0, 0, 0, 0, 0, 0b10011000, 0b00100000, 0 ;64-bit r0 code
	.gdtr:
		dw 16 ;Two descriptors long
		dq PHYSADDR(.gdt)
	
	.target64:
	
	;Alright, if we get here, we should be executing 64-bit Long Mode code.
	;Now we can actually specify addresses using a full 64-bit pointer, and jump up to our virtual space.
	bits 64
	mov RAX, .targetvm
	jmp RAX
	.targetvm:
	
	;Ah, finally. Now we're in 64-bit long mode, in virtual space.
	
	;Load the proper global descriptor table
	lgdt [cpuinit_gdtr]
	
	;Activate the proper data segment descriptors
	mov AX, (cpuinit_gdt.r0data64 - cpuinit_gdt)
	mov SS, AX
	mov DS, AX
	mov ES, AX
	mov FS, AX
	mov GS, AX
	
	;Build the Interrupt Descriptor Table using the interrupt vectors from link-time.
	mov RSI, cpuinit_isrptrs ;Read location of ISRs
	mov RDI, cpuinit_idt ;Write to interrupt descriptor table
	mov ECX, 256 ;Fill all 256 entries
	.idt_loop:
		mov RAX, [RSI] ;Load address of function to call
		mov [RDI + 0], AX ;Target offset 15..0
		
		shr RAX, 16
		mov [RDI + 6], AX ;Target offset 31..16
		
		shr RAX, 16
		mov [RDI + 8], EAX ;Target offset 64..32
		
		mov AX, cpuinit_gdt.r0code64 - cpuinit_gdt
		mov [RDI + 2], AX ;Target selector (always kernel code segment)
		
		mov AL, 0
		mov [RDI + 4], AL ;Interrupt Stack Table entry to use (don't)
		
		mov AL, 0b10001110 ;Present (1), DPL=0 (00), always 0, type = interrupt gate (1110)
		mov [RDI + 5], AL
		
		add RSI, 8 ;Advance to next 64-bit function pointer
		add RDI, 16 ;Advance to next 16-byte descriptor
		loop .idt_loop ;Counts down ECX
	
	.idt_done:
	
	;Activate the interrupt descriptor table
	lidt [cpuinit_idtr]
	
	;Use early-stack while setting up
	mov RSP, cpuinit_initstack.top
	
	;Turn on WRGSBASE and friends
	mov RAX, CR4
	or RAX, (1<<16) ;set FSGSBASE bit
	mov CR4, RAX
	
	;Set up our frame allocator using the memory map information from multiboot
	extern frame_free_multiboot
	call frame_free_multiboot
	
	;Set up keyboard support (todo - need a real driver model at some point)
	extern pic8259_init
	call pic8259_init
	extern ps2kbd_init
	call ps2kbd_init

	;Initialize kernel while single-threaded
	extern kentry_boot
	call kentry_boot
	
	;Now we need to release the other CPU cores.
	
	;Turn on the APIC Enable flag in the APIC Base Address Register MSR
	mov ECX, 0x0000001B ;APIC Base Address Register MSR
	rdmsr
	or EAX, 1 << 11 ;Set AE (APIC Enable) bit
	wrmsr
	
	;Read the base address returned from the MSR
	and EAX, 0xFFFFF000
	and EDX, 0x000FFFFF
	shl RDX, 32
	or RAX, RDX
	
	;Map a page at the top of kernel space as-linked to access the LAPIC
	or RAX, 3 ;Present, writable
	mov [cpuinit_pt + (8*511)], RAX
	mov RAX, _KSPACE_BASE + (4096*511)
	mov [cpuinit_lapicaddr], RAX
	
	;Identity-map the 2nd page of memory (0x1000-0x1FFF) for non-bootstrap cores to land in
	mov RAX, SMP_STUB_ADDR
	or RAX, 3
	mov [cpuinit_pt + ((SMP_STUB_ADDR / 4096) * 8)], RAX
	
	;Copy our trampoline into low memory for the secondary cores
	mov RCX, cpuinit_smpstub.end - cpuinit_smpstub
	mov RSI, cpuinit_smpstub
	mov RDI, SMP_STUB_ADDR
	rep movsb
	
	;Get our LAPIC address, as mapped virtually earlier
	mov RBX, [cpuinit_lapicaddr]
	
	;Clear APIC errors
	mov EAX, 0
	mov [RBX + 0x280], EAX
	
	;Send an INIT IPI to all remote cores
	mov EAX, 0xC4500 ;Init IPI, positive edge-trigger, to all-except-self
	mov [RBX + 0x300], EAX
	
	;Wait for delivery
	.init_wait:
	pause
	mov EAX, [RBX + 0x300]
	and EAX, 1<<12
	jnz .init_wait
	
	;Send a STARTUP IPI to all remote cores.
	mov EAX, 0xC4600 ;Startup IPI, positive edge-trigger, to all-except-self
	or EAX, SMP_STUB_ADDR / 4096 ;Page number to start executing
	mov [RBX + 0x300], EAX
	
	;Wait for delivery
	.sipi_wait:
	pause
	mov EAX, [RBX + 0x300]
	and EAX, 1<<12
	jnz .sipi_wait
	
	;Wait a moment before sending a second STARTUP IPI
	mov CX, 65535
	.second_sipi_delay:
	loop .second_sipi_delay
	
	mov [RBX + 0x300], EAX
	
	;Join the other cores in the common code path
	jmp cpuinit_all
	
cpuinit_fail:
	.spin:
	jmp .spin
	
;Stub copied to low memory to set up non-bootstrap cores
cpuinit_smpstub:
	
	;This code gets copied to an arbitrary low address in conventional memory.
	;Non-bootstrap cores will enter this code in Real Mode.
	;The kernel pagetables are already set up and this region is identity-mapped.
	;So we just need to kick the processor up into 64-bit mode.
	bits 16
	cli
	
	;Load a temporary GDT and enter protected mode
	lgdt [.tempgdtr32 + SMP_STUB_ADDR - cpuinit_smpstub]
	mov EAX, CR0
	or EAX, 1 ;Set PE
	mov CR0, EAX
	
	;Longjump to activate 32-bit code, using descriptors in the temporary GDT
	jmp (8) : (.target32 + SMP_STUB_ADDR - cpuinit_smpstub)
	
	.tempgdt32:
		dq 0 ;Null descriptor
		db 0xFF, 0xFF, 0x00, 0x00, 0x00, 0b10011010, 0b11001111, 0x00 ;Ring-0 32-bit code
		db 0xFF, 0xFF, 0x00, 0x00, 0x00, 0b10010010, 0b11001111, 0x00 ;Ring-0 32-bit data
	
	.tempgdtr32:
		dw 23 ;Limit
		dd .tempgdt32 + SMP_STUB_ADDR - cpuinit_smpstub ;Base
		
	.target32:
	bits 32
	
	;So now we're in 32-bit protected mode.
	;Setup now looks similar to the bootstrap core, once it's come from Multiboot and set up the pagetables.
	
	;Enable Physical Address Extension in CR4, required for long-mode
	mov EAX, CR4
	or EAX, (1<<5) ;PAE
	mov CR4, EAX
	
	;Load Page Directory Base Register with physical address of top-level paging structure (PML4) including identity-mapping
	mov EAX, PHYSADDR(cpuinit_pml4)
	mov CR3, EAX
	
	;Set Long Mode Enabled in the Extended Feature Enable Register, so we get into Long Mode when activating paging
	mov ECX, 0xC0000080 ;EFER
	rdmsr
	or EAX, (1<<8) ;LME
	wrmsr
	
	;Enable paging; Long Mode is activated in doing so because LME was set.
	mov EAX, CR0
	or EAX, (1<<31) ;PG
	mov CR0, EAX
	
	;We're down in conventional memory, still, in an identity-mapped page.
	;Now we just need to jump up into the kernel's virtual space.
	;Same problem as on the bootstrap core, though - we can't get there unless we're already in 64-bit mode.
	;Set up another temporary GDT and use it to get into 64-bit code.

	lgdt [.tempgdtr64 + SMP_STUB_ADDR - cpuinit_smpstub]
	jmp (8) : .target64 + SMP_STUB_ADDR - cpuinit_smpstub
	
	.tempgdt64:
		dq 0 ;Null
		db 0, 0, 0, 0, 0, 0b10011000, 0b00100000, 0 ;64-bit code
			
	.tempgdtr64:
		dw 15 ;Limit
		dd .tempgdt64 + SMP_STUB_ADDR - cpuinit_smpstub ;Base
	
	.target64:
	bits 64
	
	;And now that we're in 64-bit mode, we can see the whole address space, and proceed into the kernel.
	mov RAX, cpuinit_all
	jmp RAX

	.end:
	bits 64	
	
;Entry code run on all cores
align 16
cpuinit_all:

	;Load the proper descriptor tables
	lgdt [cpuinit_gdtr]
	lidt [cpuinit_idtr]
	
	;Activate the proper data segment descriptors
	mov AX, (cpuinit_gdt.r0data64 - cpuinit_gdt)
	mov SS, AX
	mov DS, AX
	mov ES, AX
	mov FS, AX
	mov GS, AX
	
	;Turn on WRGSBASE and friends
	mov RAX, CR4
	or RAX, (1<<16) ;set FSGSBASE bit
	mov CR4, RAX

	;Find a core number for ourselves.
	mov RAX, 0 ;ID to try taking, if it's the next-ID
	mov RCX, 1 ;Next-ID then stored if we are successful
	.id_loop:
		lock cmpxchg qword [cpuinit_nextid], RCX
		jz .id_done
		mov RAX, RCX
		inc RCX
		jmp .id_loop
	.id_done:
	
	;Got our unique ID in RAX. Set aside in RBX as well.
	mov RBX, RAX
	
	;Wait for any processors ahead of us to finish init.
	.wait_loop:
		mov RCX, [cpuinit_coresdone]
		cmp RCX, RAX
		je .wait_done
		pause
		jmp .wait_loop
	.wait_done:
	
	;Use the init-stack while initializing (one at a time!)
	mov RSP, cpuinit_initstack.top
	
	;Allocate a page for our task state segment
	call cpuinit_bump_alloc
	mov RSI, RAX
	
	;Set aside a pointer to it, for easier access later
	mov [cpuinit_tssptrs + (8 * RBX)], RAX
	
	;Compute where we'll store our Task State Segment Descriptor
	mov RDI, cpuinit_gdt.ktss_array ;Start of per-CPU TSS descriptors
	mov ECX, EBX ;Index of our CPU
	shl ECX, 4 ;16 bytes per TSS descriptor
	add RDI, RCX ;Advance to position in TSS descriptor array
	
	;Build the TSS descriptor
	mov AX, 0xFFF
	mov [RDI + 0], AX ;Segment limit 15..0
	
	mov RAX, RSI
	mov [RDI + 2], AX ;Segment base 15..0
	
	shr RAX, 16
	mov [RDI + 4], AL ;Segment base 23..16
	
	mov [RDI + 7], AH ;Segment base 31..24
	
	shr RAX, 16
	mov [RDI + 8], EAX ;Segment base 63..32
	
	mov EAX, 0
	mov [RDI + 12], EAX ;High dword all 0
	
	mov AL, 0b10001001 ;Present (1), DPL=0 (00), always 0, available 64-bit TSS (1001)
	mov [RDI + 5], AL
	
	mov AL, 0b00000000 ;Granularity = x1B (0), unused (00), available for OS (0), segment limit 19..16 (0000)
	mov [RDI + 6], AL
	
	;Activate it
	mov RAX, RDI
	sub RAX, cpuinit_gdt
	ltr AX
	
	;Turn on SYSCALL/SYSRET
	mov ECX, 0xC0000080 ;EFER
	rdmsr
	or EAX, (1<<0) ;set SCE bit
	wrmsr
	
	;Set up usage of SYSCALL/SYSRET
	mov ECX, 0xC0000081 ;STAR register
	mov EDX, (cpuinit_gdt.r3dummy - cpuinit_gdt) ;SYSRET CS and SS (+16 and +8 from this)
	shl EDX, 16
	mov DX, (cpuinit_gdt.r0code64 - cpuinit_gdt) ;SYSCALL CS and SS (+0 and +8 from this)
	mov EAX, 0 ;32-bit SYSCALL target EIP
	wrmsr
	
	mov ECX, 0xC0000082 ;LSTAR register
	mov RAX, cpuinit_syscall
	mov RDX, RAX ;Need to put 64-bit address in EDX:EAX rather than RAX.
	shr RDX, 32
	wrmsr
	
	mov ECX, 0xC0000083 ;CSTAR register
	mov RAX, 0 ;Don't support compatibility-mode.
	mov RDX, 0
	wrmsr 
	
	mov ECX, 0xC0000084 ;SFMASK
	mov EAX, 0xFFFFFFFF ;Mask all flags.
	mov EDX, 0 ;Reserved, RAZ
	wrmsr
	
	
	;Allocate space for our rescheduling stack and use that, instead of the shared initial stack
	call cpuinit_bump_alloc
	mov RSP, RAX
	add RSP, 4096
	
	;Now we're done with shared init resources - allow other cores to go
	inc qword [cpuinit_coresdone]
	
	;Enter the kernel
	extern kentry_sched
	call kentry_sched
	
	;Kernel shouldn't return
	lidt [0]
	int 0
	jmp 0
	

;Simple bump-allocator, which allocates single pages of kernel space after the kernel as-linked.
;Used to allocate per-CPU stacks and task-state segments.
cpuinit_bump_alloc:
	
	;Get a frame from the frame allocator
	extern hal_frame_alloc
	call hal_frame_alloc
	
	;Stick it after other allocations
	extern pt_set ;pt_set(uint64_t pml4, uint64_t addr, uint64_t frame, uint64_t flags)
	mov RDI, PHYSADDR(cpuinit_pml4)
	mov RSI, [cpuinit_bump_next]
	mov RDX, RAX
	mov RCX, 3
	call pt_set
	
	;Advance by two frames to leave a guard page before following allocations
	mov RAX, [cpuinit_bump_next]
	push RAX
	
	add RAX, 8192
	mov [cpuinit_bump_next], RAX
	
	pop RAX
	ret
	
;Returns the location of the calling CPU's task-state segment in RAX. Only alters RAX.
align 16
global cpuinit_gettss
cpuinit_gettss:
	;Find our CPU index based on our task register
	mov RAX, 0
	str AX ;Find our current task-state selector
	sub AX, (cpuinit_gdt.ktss_array - cpuinit_gdt) ;Make relative to the first task-state selector
	shr AX, 4 ;Divide by 16 to find out which, in the array, it selects. That's our CPU number.
	
	;Get our task-state segment location from the array of TSS pointers we set aside
	mov RAX, [cpuinit_tssptrs + (8 * RAX)]
	ret

;Called to exit the kernel initially to a nearly-undefined user state	
align 16
global hal_exit_fresh ;void hal_exit_fresh(uintptr_t u_pc, void *k_sp);
hal_exit_fresh:

	;Save k_sp parameter as RSP0 entry in task-state segment
	call cpuinit_gettss
	mov [RAX + 0x4], RSI
	
	;Set up for dropping to usermode with a sysret
	mov RCX, RDI ;Will be RIP - user entry point
	mov R11, 0x202 ;Will be RFLAGS - initialize to just interrupts (and always-1 flag) enabled
	
	;Zero all other registers
	mov RAX, 0
	mov RDX, RAX
	mov RBX, RAX
	mov RBP, RAX
	mov RSI, RAX
	mov RDI, RAX
	mov R8,  RAX
	mov R9,  RAX
	mov R10, RAX
	mov R12, RAX
	mov R13, RAX
	mov R14, RAX
	mov R15, RAX
	
	;Drop to user-mode
	cli ;Disable interrupts so we don't get caught with wrong GS-base
	swapgs ;Preserve kernel GS-base
	o64 a64 sysret ;Drop to usermode
	
align 16
global hal_intr_wake ;void hal_intr_wake(void);
hal_intr_wake:
	;Send a broadcast IPI on our "wakeup" interrupt vector (255)
	mov RCX, [cpuinit_lapicaddr]
	mov EAX, 0xC45FF ;Fixed interrupt, positive edge-trigger, to all-except-self, vector 0xFF
	mov [RCX + 0x300], EAX
	ret

;Entry point for system calls
bits 64
align 16
cpuinit_syscall:

	;Interrupts should be disabled at this point - we clear all flags on syscall.
	;Swap to kernel-mode GS.
	swapgs
		
	;Load the RSP0 (kernel stack pointer) from the task state segment for this CPU.
	;We can clobber RAX to do this, because it's neither saved nor involved in passing parameters.
	call cpuinit_gettss
	mov RAX, [RAX + 0x4]
	xchg RAX, RSP ;Set user's RSP aside in RAX and place kernel stack pointer in RSP.
	
	;Build a system-call exit buffer on the stack.
	;The kernel expects the first 4 entries to always be size, return address, return stack, return value.
	sub RSP, (8*11)
	mov qword [RSP + (8*0)], 8*11 ;Total size of the buffer
	mov [RSP + (8*1)], RCX ;User return address
	mov [RSP + (8*2)], RAX ;User stack pointer
	mov qword [RSP + (8*3)], -38 ;User return value (-ENOSYS)
	mov [RSP + (8*4)], RBP
	mov [RSP + (8*5)], RBX
	mov [RSP + (8*6)], R12
	mov [RSP + (8*7)], R13
	mov [RSP + (8*8)], R14
	mov [RSP + (8*9)], R15
	swapgs
	rdgsbase RAX
	swapgs
	mov [RSP + (8*10)], RAX ;User GS-base	
	
	;Pull RCX/4th parameter out of R10, where it was stashed before the SYSCALL instruction
	mov RCX, R10
	
	;Pass the exit buffer's location on the stack as the 7th parameter.
	push RSP
	
	;Other parameters are already in registers. Handle the system call.
	;Note that we just trash the saved flags (R11) as flags aren't preserved across calls in SysVABI.
	extern kentry_syscall
	call kentry_syscall
	
	;The kernel should call a function to exit, rather than returning.
	.spin:
	mov RDI, .panicstr
	call hal_panic
	jmp .spin
	.panicstr:
		db "syscall botched exit\0"

;Called to exit the kernel from a system call or exception
align 16
global hal_exit_resume ;void hal_exit_resume(hal_exit_t *e, void *k_sp);
hal_exit_resume:
		
	;Disable interrupts so we don't get caught with the wrong GS-base
	cli
	
	;Swap to user GS
	swapgs
	
	;Set aside desired kernel stack pointer in RSP0 field of TSS.
	call cpuinit_gettss
	mov [RAX + 0x4], RSI
	
	;Check how long the exit-buffer is.
	;Resume based on it.
	cmp qword [RDI], 8*20
	je .from_exception
	cmp qword [RDI], 8*11
	je .from_syscall
	
	;Bad size in exit buffer
	mov RDX, RDI
	mov RDI, .badstr
	jmp hal_panic
	.badstr:
		db "bad exit-buffer size\0"
	
	;Restoring an exit-buffer that was made on a system-call
	.from_syscall:
		;Restore saved registers
		mov RCX, [RDI + (8*1)] ;User return address (restored to RIP by sysret)
		mov RSP, [RDI + (8*2)] ;User stack pointer
		mov RAX, [RDI + (8*3)] ;User return value
		mov RBP, [RDI + (8*4)]
		mov RBX, [RDI + (8*5)]
		mov R12, [RDI + (8*6)]
		mov R13, [RDI + (8*7)]
		mov R14, [RDI + (8*8)]
		mov R15, [RDI + (8*9)]
		
		mov R11, [RDI + (8*10)] ;User GS-base
		wrgsbase R11
		
		;Clobber flags because those don't get preserved
		mov R11, 0x202 ;User flags (restored to RFLAGS by sysret)
		o64 a64 sysret
	
	;Restoring an exit-buffer that was made on an exception
	.from_exception:
		
		;Restore GS-base
		mov R11, [RDI + (8* 4)] ;User GS-base
		wrgsbase R11
		
		;Restore general purpose registers, except RDI, which contains our exit-buffer location
		mov RCX, [RDI + (8* 6)]
		mov RDX, [RDI + (8* 7)]
		mov RBX, [RDI + (8* 8)]
		mov RBP, [RDI + (8* 9)]
		mov RSI, [RDI + (8*10)]
		mov R8 , [RDI + (8*12)]
		mov R9 , [RDI + (8*13)]
		mov R10, [RDI + (8*14)]
		mov R11, [RDI + (8*15)]
		mov R12, [RDI + (8*16)]
		mov R13, [RDI + (8*17)]
		mov R14, [RDI + (8*18)]
		mov R15, [RDI + (8*19)]
		
		;Note that we're still on the old kernel-stack here, because we didn't restore RSP.
		;Put RIP, CS, RFLAGS, RSP, SS on the kernel stack for later IRETQ
		push qword (cpuinit_gdt.r3data64 - cpuinit_gdt) | 3 ;SS
		push qword [RDI + (8* 2)] ;RSP
		push qword [RDI + (8* 5)] ;RFLAGS
		push qword (cpuinit_gdt.r3code64 - cpuinit_gdt) | 3 ;CS
		push qword [RDI + (8* 1)] ;RIP
		
		;We've read everything but RDI out of the exit-buffer now.
		;Clobber RDI, the location of the buffer, in restoring it
		mov RDI, [RDI + (8*11)]
		
		;Restore the last few registers with an IRETQ from the kernel stack
		iretq ;Pops RIP, CS, RFLAGS, RSP, SS
	

;Exception handlers
bits 64

;Divide-by-zero exception (vector 0)
cpuinit_isr_de:
	push qword 0 ;phony error-code
	push qword 0 ;vector
	jmp cpuinit_exception

;Debug exception (vector 1)
cpuinit_isr_db:
	push qword 0 ;phony error-code
	push qword 1 ;vector
	jmp cpuinit_exception

;Nonmaskable interrupt (vector 2)
cpuinit_isr_nmi:
	push qword 0 ;phony error-code
	push qword 2 ;vector
	jmp cpuinit_exception

;Breakpoint exception (vector 3)
cpuinit_isr_bp:
	push qword 0 ;phony error-code
	push qword 3 ;vector
	jmp cpuinit_exception

;Overflow exception (vector 4)
cpuinit_isr_of:
	push qword 0 ;phony error-code
	push qword 4 ;vector
	jmp cpuinit_exception

;Bound range exception (vector 5)
cpuinit_isr_br:
	push qword 0 ;phony error-code
	push qword 5 ;vector
	jmp cpuinit_exception

;Invalid opcode exception (vector 6)
cpuinit_isr_ud:
	push qword 0 ;phony error-code
	push qword 6 ;vector
	jmp cpuinit_exception

;Device not available exception (vector 7)
cpuinit_isr_nm:
	push qword 0 ;phony error-code
	push qword 7 ;vector
	jmp cpuinit_exception

;Double fault (vector 8)
cpuinit_isr_df:
	mov RDI, .str
	jmp hal_panic
	.str:
	db "cpuinit_isr_df", 0

;Coprocessor segment overrun (vector 9)
cpuinit_isr_cop:
	mov RDI, .str
	jmp hal_panic
	.str:
	db "cpuinit_isr_cop", 0

;Invalid TSS (vector 10)
cpuinit_isr_ts:
	;CPU pushes error-code
	push qword 10 ;vector
	jmp cpuinit_exception

;Segment not present (vector 11)
cpuinit_isr_np:
	;CPU pushes error-code
	push qword 11 ;vector
	jmp cpuinit_exception

;Stack exception (vector 12)
cpuinit_isr_ss:
	;CPU pushes error-code
	push qword 12 ;vector
	jmp cpuinit_exception

;General protection exception (vector 13)
cpuinit_isr_gp:
	;CPU pushes error-code
	push qword 13 ;vector
	jmp cpuinit_exception

;Page fault exception (vector 14)
cpuinit_isr_pf:
	;CPU pushes error-code
	push qword 14 ;vector
	jmp cpuinit_exception

;x87 floating-point exception (vector 16)
cpuinit_isr_mf:
	push qword 0 ;phony error-code
	push qword 16 ;vector
	jmp cpuinit_exception

;Alignment check exception (vector 17)
cpuinit_isr_ac:
	;CPU pushes error-code
	push qword 17 ;vector
	jmp cpuinit_exception

;Machine-check exception (vector 18)
cpuinit_isr_mc:
	mov RDI, .str
	jmp hal_panic
	.str:
	db "cpuinit_isr_mc", 0

;SIMD floating-point exception (vector 19)
cpuinit_isr_xf:
	push qword 0 ;phony error-code
	push qword 19 ;vector
	jmp cpuinit_exception

;Hypervisor injection exception (vector 28)
cpuinit_isr_hv:
	mov RDI, .str
	jmp hal_panic
	.str:
	db "cpuinit_isr_hv", 0

;VMM communication exception (vector 29)
cpuinit_isr_vc:
	mov RDI, .str
	jmp hal_panic
	.str:
	db "cpuinit_isr_vc", 0

;Security exception (vector 30)
cpuinit_isr_sx:
	mov RDI, .str
	jmp hal_panic
	.str:
	db "cpuinit_isr_sx", 0

;Entry point for unused interrupt vectors
cpuinit_isr_bad:
	mov RDI, .str
	jmp hal_panic
	.str:
	db "cpuinit_isr_bad", 0
	
	
;Common handling for exceptions
cpuinit_exception:

	
	;CPU should have already switched us to the kernel stack, as we store it in the TSS when dropping to user-mode.
	;Interrupts will already be disabled.
	;The stack contains already: Vector number, error-code, RIP, CS, RFLAGS, RSP, SS
	
	;Push the rest of the registers on the stack.
	push R15
	push R14
	push R13
	push R12
	push R11		
	push R10
	push R9
	push R8
	push RDI
	push RSI
	push RBP
	push RBX
	push RDX
	push RCX	
	
	mov R11, [RSP + (8*18)] ;Get RFLAGS that the CPU pushed
	push R11
	
	rdgsbase R11
	push R11
	swapgs ;For now just assume that we were in usermode, and flip to the kernel GS-base
	
	;Kernel expects the first 4 entries of the exit buffer to be size, RIP, RSP, RAX
	push RAX
	
	mov R11, [RSP + (8*22)] ;Get RSP that the CPU pushed
	push R11
	
	mov R11, [RSP + (8*20)] ;Get RIP that the CPU pushed
	push R11
	
	push qword 20*8 ;Size of exit buffer including this qword

	;Alright, exit buffer is built on the stack.
	
	;Figure out what signal number to tell the kernel.
	mov RDI, [RSP+(8*20)] ;Get vector number that we pushed before jumping to cpuinit_exception
	extern excsig
	call excsig
	
	mov RDI, RAX ;Return value from excsig
	mov RSI, [RSP+(8*22)] ;RIP that CPU pushed
	mov RDX, CR2 ;Fault address from CPU
	mov RCX, RSP ;Location of exit-buffer
	extern kentry_exception ;void kentry_exception(int signum, uint64_t pc_addr, uint64_t ref_addr, hal_exit_t *eptr)
	call kentry_exception
	
	;Exception handling in kernel should wipe-out kernel stack.
	.spin:
	hlt
	jmp .spin


;Interrupt service routines for legacy interrupts
cpuinit_isr_irq0:
	push qword 0
	jmp cpuinit_irq
	
cpuinit_isr_irq1:
	push qword 1
	jmp cpuinit_irq
	
cpuinit_isr_irq2:
	push qword 2
	jmp cpuinit_irq
	
cpuinit_isr_irq3:
	push qword 3
	jmp cpuinit_irq
	
cpuinit_isr_irq4:
	push qword 4
	jmp cpuinit_irq
	
cpuinit_isr_irq5:
	push qword 5
	jmp cpuinit_irq
	
cpuinit_isr_irq6:
	push qword 6
	jmp cpuinit_irq
	
cpuinit_isr_irq7:
	push qword 7
	jmp cpuinit_irq
	
cpuinit_isr_irq8:
	push qword 8
	jmp cpuinit_irq
	
cpuinit_isr_irq9:
	push qword 9
	jmp cpuinit_irq
	
cpuinit_isr_irq10:
	push qword 10
	jmp cpuinit_irq
	
cpuinit_isr_irq11:
	push qword 11
	jmp cpuinit_irq
	
cpuinit_isr_irq12:
	push qword 12
	jmp cpuinit_irq
	
cpuinit_isr_irq13:
	push qword 13
	jmp cpuinit_irq
	
cpuinit_isr_irq14:
	push qword 14
	jmp cpuinit_irq
	
cpuinit_isr_irq15:
	push qword 15
	jmp cpuinit_irq
	
	
;Common handling for IRQs
cpuinit_irq:
	;This is simpler than exception handling because we'll never context-switch away.
	;So we'll always finish this on the same CPU we started, and can be minimally invasive.
	;Save all registers that the kernel might use but won't save when making function calls.
	push RAX
	push RDI
	push RSI
	push RDX
	push RCX
	push R8
	push R9
	push R10
	push R11
	
	rdgsbase RAX
	push RAX
	
	;Clear the GS-base as interrupts don't execute "on a thread"
	mov RAX, 0
	wrgsbase RAX
	
	;Check interrupt number
	mov RAX, [RSP+(8*10)]
	cmp RAX, 1
	jne .done
		;IRQ 1 - keyboard
		extern ps2kbd_isr
		call ps2kbd_isr
	.done:
	
	;Send EOI
	mov RDI, [RSP+(8*10)]
	extern pic8529_pre_iret
	call pic8529_pre_iret
	
	pop RAX
	wrgsbase RAX
	
	pop R11
	pop R10
	pop R9
	pop R8
	pop RCX
	pop RDX
	pop RSI
	pop RDI
	pop RAX
	
	add RSP, 8 ;Pop vector number
	
	iretq

	
;Interrupt service routine for "wakeup" interrupts
bits 64
align 16
cpuinit_isr_woke:
	;Do nothing - we've been brought out of a halt, and that's all we care about.
	iretq
	
section .data
bits 32

;Global descriptor table
align 16
cpuinit_gdt:
	
	;Null descriptor
	dq 0
	
	;!!!QEMU DISCREPANCY!!!
	;The AMD64 reference (24593 r3.35 p90) says that all fields except "P" are ignored in long-mode data segment descriptors.
	;However, QEMU requires that the "writable" bit be set on a data segment descriptor if it is selected for SS.
	
	;Note that a SYSCALL instruction will load CS and SS with a certain value +0 and +8.
	;So the order of these two descriptors matters - r0data64 must follow r0code64.
	
	;Kernel code segment (64-bit long mode, ring-0)
	.r0code64:
	db 0 ;Ignored
	db 0 ;Ignored
	db 0 ;Ignored
	db 0 ;Ignored
	db 0 ;Ignored
	db 0b10011000 ;Present (1), DPL=0 (00), code segment (11), non-conforming (0), ignored (00)
	db 0b00100000 ;Ignored (0), 64-bit (01), unused (0), ignored (0000)
	db 0 ;Ignored
	
	;Kernel data segment (ring-0)
	.r0data64:
	db 0 ;Ignored
	db 0 ;Ignored
	db 0 ;Ignored
	db 0 ;Ignored
	db 0 ;Ignored
	db 0b10010010 ;Present (1), ignored (00), data segment (10), ignored (0), writable (1), ignored (0)
	db 0 ;Ignored
	db 0 ;Ignored
	
	
	;Similarly, SYSRET loads CS and SS with a certain value +16 and +8.
	;The descriptor that SYSRET points to, +0, is used for returning to 32-bit code. We don't support that.
	;So we must have r3dummy, r3data64, and then r3code64.
	.r3dummy:
	dq 0 ;Don't support returning to 32-bit mode
	
	;!!!QEMU DISCREPANCY!!!
	;The AMD64 reference (24593 r3.35 p91) says privilege level on a data segment is ignored.
	;It explicitly states "a data-segment-descriptor DPL field is ignored in 64-bit mode" (24593 r3.35 p92).
	;QEMU seems to check the privilege level on this when we use it in an iretq though.
	
	;User data segment (ring-3)
	.r3data64:
	db 0 ;Ignored
	db 0 ;Ignored
	db 0 ;Ignored
	db 0 ;Ignored
	db 0 ;Ignored
	db 0b11110010 ;Present (1), SHOULD BE IGNORED (11), data segment (10), ignored (0), writable (1), ignored (0)
	db 0 ;Ignored
	db 0 ;Ignored
	
	;User code segment (64-bit long mode, ring-3)
	.r3code64:
	db 0 ;Ignored
	db 0 ;Ignored
	db 0 ;Ignored
	db 0 ;Ignored
	db 0 ;Ignored
	db 0b11111000 ;Present (1), DPL=3 (11), code segment (11), non-conforming (0), ignored (00)
	db 0b00100000 ;Ignored (0), 64-bit (01), unused (0), ignored (0000)
	db 0 ;Ignored
	
	;Descriptors for task-state segments for each CPU
	;Built at runtime, so we can swizzle the address bytes
	align 16
	.ktss_array:
	times (16 * CPU_MAX) db 0 ;Each descriptor is 16 bytes, and we need one per CPU
	.end:

;Pointer to global descriptor table for LGDT instruction
align 16
cpuinit_gdtr:
	dw (cpuinit_gdt.end - cpuinit_gdt) - 1
	dq cpuinit_gdt
	
;Pointer to interrupt descriptor table for LIDT instruction
align 16
cpuinit_idtr:
	dw (cpuinit_idt.end - cpuinit_idt) - 1
	dq cpuinit_idt
	
;Function pointers we use for building the interrupt descriptor table
align 16
cpuinit_isrptrs:
	;First 32 vectors - CPU exceptions
	dq cpuinit_isr_de  ;0
	dq cpuinit_isr_db  ;1
	dq cpuinit_isr_nmi ;2
	dq cpuinit_isr_bp  ;3
	dq cpuinit_isr_of  ;4
	dq cpuinit_isr_br  ;5
	dq cpuinit_isr_ud  ;6
	dq cpuinit_isr_nm  ;7
	dq cpuinit_isr_df  ;8
	dq cpuinit_isr_cop ;9
	dq cpuinit_isr_ts  ;10
	dq cpuinit_isr_np  ;11
	dq cpuinit_isr_ss  ;12
	dq cpuinit_isr_gp  ;13
	dq cpuinit_isr_pf  ;14
	dq cpuinit_isr_bad ;15
	dq cpuinit_isr_mf  ;16
	dq cpuinit_isr_ac  ;17
	dq cpuinit_isr_mc  ;18
	dq cpuinit_isr_xf  ;19
	dq cpuinit_isr_bad ;20
	dq cpuinit_isr_bad ;21
	dq cpuinit_isr_bad ;22
	dq cpuinit_isr_bad ;23
	dq cpuinit_isr_bad ;24
	dq cpuinit_isr_bad ;25
	dq cpuinit_isr_bad ;26
	dq cpuinit_isr_bad ;27
	dq cpuinit_isr_hv  ;28
	dq cpuinit_isr_vc  ;29
	dq cpuinit_isr_sx  ;30
	dq cpuinit_isr_bad ;31
	
	;Vectors 32-47 used for legacy IRQs
	dq cpuinit_isr_irq0 ;32
	dq cpuinit_isr_irq1 ;33
	dq cpuinit_isr_irq2 ;34
	dq cpuinit_isr_irq3 ;35
	dq cpuinit_isr_irq4 ;36
	dq cpuinit_isr_irq5 ;37
	dq cpuinit_isr_irq6 ;38
	dq cpuinit_isr_irq7 ;39
	dq cpuinit_isr_irq8 ;40
	dq cpuinit_isr_irq9 ;41
	dq cpuinit_isr_irq10 ;42
	dq cpuinit_isr_irq11 ;43
	dq cpuinit_isr_irq12 ;44
	dq cpuinit_isr_irq13 ;45
	dq cpuinit_isr_irq14 ;46
	dq cpuinit_isr_irq15 ;47
	
	;Other vectors unused
	times 207 dq cpuinit_isr_bad ;48-254
	
	;Vector 255 is wakeup
	dq cpuinit_isr_woke ;255


;Simple bump allocator for allocating per-core structures, virtually, after kernel as-linked
align 8
cpuinit_bump_next:
	dq _KERNEL_END + 4096

section .bss
bits 32

;Stack for initial single-processor setup of kernel
align 4096
cpuinit_initstack:
	resb 4096
	.top:

;PML4 (top level paging structure) for kernel as linked
align 4096
global cpuinit_pml4
cpuinit_pml4:
	resb 4096
	
;Page directory pointer table for kernel as linked
align 4096
global cpuinit_pdpt
cpuinit_pdpt:
	resb 4096

;Page directory for kernel as linked
align 4096
global cpuinit_pd
cpuinit_pd:
	resb 4096

;Page table for kernel as linked
align 4096
global cpuinit_pt
cpuinit_pt:
	resb 4096
	
;Space for interrupt descriptor table (IDT)
;We have to build this at runtime because of the crazy byte swizzling needed.
align 4096
cpuinit_idt:
	resb 256 * 16
	.end:
	
;Address where we mapped the Local APIC
align 8
cpuinit_lapicaddr:
	resb 8

;ID numbers claimed atomically by CPUs on startup
align 8
cpuinit_nextid:
	resb 8

;Number of CPUs that have completed startup successfully
align 8
cpuinit_coresdone:
	resb 8

;Pointers to task state segments for each CPU
align 8
cpuinit_tssptrs:
	resb 8 * CPU_MAX
	

	
