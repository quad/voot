!   crt0.s
!
! DESCRIPTION
!
!   Bootstrap for np-voot drivers.
!

    .global start

start:
    nop

!
!   Handle clearing the global variable memory space.
!

clear_bss_maybe:
    mov.l   .L_edata_addr, r0
    mov.l   .L_end_addr, r1
    mov     #0, r2
.L_init_L0:
    mov.l   r2, @r0
    add     #4, r0
    cmp/ge  r0, r1
    bt      .L_init_L0

!
!   Jump to the C initialization core.
!

go:
    nop
    mov.l   main_addr, r0
    jmp     @r0
    nop

    .align  4

.L_edata_addr:
    .long   __bss_start
.L_end_addr:
    .long   _end

main_addr:
    .long   _np_initialize
