!   bswap.s
!
!   $Id: bswap.s,v 1.2 2002/06/12 00:59:42 quad Exp $
!
! DESCRIPTION
!
!   Byte swapping functions.
!
!   Since the SH4 has them in hardware, why not use it?
!

	.globl _bswap16, _bswap32

	.text

	! r4 = dest

_bswap16:
	rts	
	swap.b	r4,r0

_bswap32:
	swap.b	r4,r0
	swap.w	r0,r4
	rts
	swap.b	r4,r0
	
	.end

