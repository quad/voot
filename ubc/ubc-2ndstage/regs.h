#ifndef __REGS_H__
#define __REGS_H__

typedef struct
{
    unsigned int fpscr;
    unsigned int fpul;
    unsigned int pr;
    unsigned int mach;
    unsigned int macl;

    unsigned int vbr;
    unsigned int gbr;
    unsigned int sr;
    unsigned int dbr;

    unsigned int r7_bank;
    unsigned int r6_bank;
    unsigned int r5_bank;
    unsigned int r4_bank;
    unsigned int r3_bank;
    unsigned int r2_bank;
    unsigned int r1_bank;
    unsigned int r0_bank;

    unsigned int r14;
    unsigned int r13;
    unsigned int r12;
    unsigned int r11;
    unsigned int r10;
    unsigned int r9;
    unsigned int r8;
    unsigned int r7;
    unsigned int r6;
    unsigned int r5;
    unsigned int r4;
    unsigned int r3;
    unsigned int r2;
    unsigned int r1;
    unsigned int r0;
} register_stack;

unsigned int dbr(void);
void dbr_set(void *set);

unsigned int vbr(void);
void vbr_set(void *set);

unsigned int r15(void);
unsigned int spc(void);

#endif
