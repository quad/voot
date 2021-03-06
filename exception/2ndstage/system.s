    .section .text
    .global _vbr
    .global _vbr_set
    .global _dbr
    .global _dbr_set
    .global _r15
    .global _spc
    .global _sr
    .global _fpscr
    .global _gbr
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

_sr:
    stc     SR, r0
    rts
    nop

_fpscr:
    sts     FPSCR, r0
    rts
    nop

_gbr:
    stc     GBR, r0
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
