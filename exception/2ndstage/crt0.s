    .global start

start:
    nop

clear_bss_maybe:
    mov.l   .L_edata_addr, r0
    mov.l   .L_end_addr, r1
    mov     #0, r2
.L_init_L0:
    mov.l   r2, @r0
    add     #4, r0
    cmp/ge  r0, r1
    bt      .L_init_L0

go:
    nop
    mov.l   main_addr, r0
    jmp     @r0
    nop

    .align  4
main_addr:
    .long   _dc_main
.L_edata_addr:
        .long   __bss_start
.L_end_addr:
        .long   _end
