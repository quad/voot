!   system.s
!
!DESCRIPTION
!   Byte-swapping routines for the network modules.
!
!COPYING
!   See "COPYING" in the root directory of the distribution.
!
!CHANGELOG
!   Mon Aug  6 15:46:17 PDT 2001    Scott Robinson <scott_dcdev@dsn.itgo.com>
!       Imported, modified, and just generally added a timestamp when I
!       created the libdc distribution.
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

