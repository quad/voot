!   system.s
!
!DESCRIPTION
!   A variety of direct-to-bone SH4 accessor functions.
!
!COPYING
!   See "COPYING" in the root directory of the distribution.
!
!CHANGELOG
!   Mon Aug  6 15:46:17 PDT 2001    Scott Robinson <scott_dcdev@dsn.itgo.com>
!       Imported, modified, and just generally added a timestamp when I
!       created the libdc distribution.
!

    .section .text
    .global _vbr
    .global _vbr_set
    .global _dbr
    .global _dbr_set
    .global _r15
    .global _spc
    .global _disable_cache
    .global _flush_cache

_vbr:
    stc     VBR, r0
    rts
    nop

_vbr_set:
    ldc     r4, VBR
    rts
    nop

_dbr:
    stc     DBR, r0
    rts
    nop

_dbr_set:
    ldc     r4, DBR
    rts
    nop

_r15:
    mov     r15, r0
    rts
    nop

_spc:
    stc     SPC, r0
    rts
    nop

_disable_cache:
    mov.l   disable_cache_k, r0
    mov.l   p2_mask, r1
    or      r1, r0
    jmp     @r0
    nop

disable_cache:
    mov.l    ccr_addr,r0
    mov.l    ccr_data_k,r1
    mov.l    @r1,r1
    mov.l    r1,@r0
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    rts
    nop

_flush_cache:
    mov     #-96, r3
    mov     #-1, r2
    shll8   r3
    shlr2   r2
    shll16  r3
    shlr    r2
    mova    skip1, r0
    and     r2, r0
    or      r3, r0
    jmp     @r0
    nop

    .align  4

skip1:
    stc.l   sr, @-r15
    stc     sr, r0
    or      #240, r0
    ldc     r0, sr
    mov     #-12, r5
    shll16  r5
    mov     r5, r6
    add     #32, r6
    shll8   r5
    shll8   r6
    mov     #-1, r3
    shll8   r3
    shll16  r3
    add     #28, r3
    mov.l   @r3, r0
    tst     #32, r0
    bt/s    skip2
    mov     #2, r2
    mov     #1, r2
skip2:
    shll8   r2
    shll2   r2
    shll2   r2
    mov     #0, r1
loop1:
    add     #-32, r2
    mov     r2, r0
    shlr2   r0
    shll2   r0
    tst     r2, r2
    mov.l   r1, @(r0, r6)
    mov.l   r1, @(r0, r5)
    bf      loop1
    nop
    nop
    nop
    nop
    nop
    nop

    ldc.l   @r15+, sr
    rts
    nop

    .align  4
disable_cache_k:
    .long   disable_cache
p2_mask:
    .long   0xa0000000
ccr_addr:
    .long   0xff00001c
ccr_data_k:
    .long   ccr_data
ccr_data:
    .long   0x00000808
