#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include "vars.h"

#define REG_EXPEVT      (REGISTER(uint32) (0xFF000024))
#define REG_INTEVT      (REGISTER(uint32) (0xFF000028))

#define EXP_TYPE_GEN        1
#define EXP_TYPE_CACHE      2
#define EXP_TYPE_INT        3

#define EXP_CODE_INT9       0x320
#define EXP_CODE_INT11      0x360
#define EXP_CODE_INT13      0x3A0
#define EXP_CODE_UBC        0x1E0
#define EXP_CODE_RXI        0x720
#define EXP_CODE_BAD        0xFFF

#define VRAM_START      ((uint16 *) (0xa5000000 + *(REGISTER(uint32) 0xa05f8050)))

typedef struct
{
    uint32 fpscr;
    uint32 fpul;
    uint32 pr;
    uint32 mach;
    uint32 macl;

    uint32 vbr;
    uint32 gbr;
    uint32 sr;
    uint32 dbr;

    uint32 r7_bank;
    uint32 r6_bank;
    uint32 r5_bank;
    uint32 r4_bank;
    uint32 r3_bank;
    uint32 r2_bank;
    uint32 r1_bank;
    uint32 r0_bank;

    uint32 r14;
    uint32 r13;
    uint32 r12;
    uint32 r11;
    uint32 r10;
    uint32 r9;
    uint32 r8;
    uint32 r7;
    uint32 r6;
    uint32 r5;
    uint32 r4;
    uint32 r3;
    uint32 r2;
    uint32 r1;

    uint32 exception_type;
    uint32 r0;
} register_stack;

extern void * dbr(void);
extern void dbr_set(const void *set);

extern void * vbr(void);
extern void vbr_set(const void *set);

extern uint32 r15(void);
extern uint32 spc(void);

extern void flush_cache();

#endif
