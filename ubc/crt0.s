! crt0.o for Dreamcast
! Based on the old serial example by Marcus Comstedt

.globl start

.text

start:
	! First, make sure to run in the P2 area
	mov.l	setup_cache_addr,r0
	mov.l	p2_mask,r1
	or	r1,r0
	jmp	@r0
	nop

setup_cache:
	! Now that we are in P2, it's safe
	! to enable the cache
	mov.l	ccr_addr,r0
	mov.w	ccr_data,r1
	mov.l	r1,@r0
	! After changing CCR, eight instructions
	! must be executed before it's safe to enter
	! a cached area such as P1. This will easily
	! be taken care of with setting up the stack
	! and some nops.
	sts.l	pr,@-r15		! 1
	mov.l	old_stack_addr,r0	! 2
	mov.l	r15,@r0			! 3
	mov.l	new_stack,r15		! 4
	mov.l	main_addr,r0		! 5
	mov	#0,r1			! 6
	nop				! 7
	nop				! 8
	jsr	@r0			! go
	mov	r1,r0
	nop
	! Put back the old stack pointer and PR
	mov.l	old_stack,r15
	lds.l	@r15+,pr
	rts
	nop

	.align	4
p2_mask:
	.long	0xa0000000
setup_cache_addr:
	.long	setup_cache
main_addr:
	.long	_dc_main
ccr_addr:
	.long	0xff00001c
ccr_data:
! This does something with the cache. Jules told me and I have no clue.
!	.word	0x090d
	.word	0x090c
	.word	0x0000
old_stack_addr:
	.long	old_stack + 0x20000000
old_stack:
	.long	0x00000000
new_stack:
! This decides where the stack is. I want it where DC games put it.
!   .long   0x8c00f400
    .long   0x8c008000
