! crt0.o for Dreamcast
! Based on the old serial example by Marcus Comstedt

.globl start

.text

start:
! First, make sure to run in the P2 area
    mov.l   setup_cache_addr, r0
    mov.l   p2_mask, r1
    or      r1, r0
    jmp     @r0
    nop

setup_cache:
! Now that we are in P2, it's safe
! to enable the cache
    mov.l   ccr_addr, r0
    mov.w   ccr_data, r1
    mov.l   r1, @r0

! Setup the new stack and save PR.
prep_new_stack:
    sts.l   pr, @-r15
    mov.l   old_stack_addr, r0
    mov.l   r15, @r0
    mov.l   new_stack, r15

! Prepare for the jump to the main code.
prep_main_jump:
    mov.l   main_addr, r0
    mov     #0, r1

! After changing CCR, eight instructions
! must be executed before it's safe to enter
! a cached area such as P1.
delay_for_cache:
    nop                         ! 1
    nop                         ! 2
    nop                         ! 3
    nop                         ! 4
    nop                         ! 5
    nop                         ! 6
    nop                         ! 7
    nop                         ! 8

! Jump to the main code. (Back in P1)
main_jump:
    jsr     @r0
    mov     r1, r0

! Put back the old stack pointer and PR
prep_old_stack:
!    mov.l   old_stack, r15
!    lds.l   @r15+, pr

! Return to whence we came.
    nop
    rts
    nop

    .align  4
p2_mask:
    .long   0xa0000000
setup_cache_addr:
    .long   setup_cache
ccr_addr:
    .long   0xff00001c
ccr_data:
! Andrew told me to use this one, now I don't have to worry about flushing
!  the cache.
    .word    0x090b
! This appears to work - Andrew vets it too.
!    .word    0x090c
! This is from Marcus' original example.
!    .word    0x090d
    .word   0x0000              ! Buffer so data is still aligned.

main_addr:
    .long   _main

old_stack_addr:
    .long   old_stack
old_stack:
    .long   0x12345678
new_stack:
    .long   0x8c00f400          ! The Katana default stack.
!    .long   0x8c008000
