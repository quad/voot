!   biosfont-lowlevel.s
!
!   $Id: biosfont-lowlevel.s,v 1.1 2002/06/24 00:58:04 quad Exp $
!
! DESCRIPTION
!
!   Accessors to the biosfont section of the Dreamcast BIOS.
!

_bfont_get_address:
    mov.l   syscall_b4, r0
    mov.l   @r0, r0
    jmp     @r0
    mov     #0, r1

_bfont_lock:
    mov.l   syscall_b4, r0
    mov.l   @r0, r0
    jmp     @r0
    mov     #1, r1

_bfont_unlock:
    mov.l   syscall_b4, r0
    mov.l   @r0, r0
    jmp     @r0
    mov     #2, r1

    .align 4

syscall_b4:
    .long   0x8c0000b4
