!   bswap.s
!
!   $Id: bswap.s,v 1.3 2002/11/14 20:56:02 quad Exp $
!
! DESCRIPTION
!
!   Byte swapping functions.
!
!   Since the SH4 has them in hardware, why not use it?
!

    .section .text

    .global _bswap16, _bswap32

_bswap16:
    rts    
    swap.b  r4, r0

_bswap32:
    swap.b  r4, r0
    swap.w  r0, r4
    rts
    swap.b  r4, r0
