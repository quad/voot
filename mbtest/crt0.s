! crt0.o for Dreamcast
! Based on the old serial example by Marcus Comstedt

.globl start

start:
	nop
	nop
	mov.l	main_addr,r0		! 5
	jmp	@r0			! go
	nop

	.align	4
main_addr:
	.long	_dc_main
