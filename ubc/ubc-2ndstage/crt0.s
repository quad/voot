.globl start

start:
    nop
    nop
    mov.l   main_addr, r0
    jmp     @r0
    nop

    .align	4
main_addr:
    .long   _dc_main
