/*  system.h

    $Id: system.h,v 1.5 2002/06/23 03:22:52 quad Exp $

*/

#ifndef __COMMON_SYSTEM_H__
#define __COMMON_SYSTEM_H__

#include "vars.h"

/* NOTE: The UBC register bases. */

#define _UBC_BASE       (0xff200000)
#define _UBC_ASID_BASE  (0xff000000)

/*
    NOTE: Abbreviation expansion...

    BAR?    - Break Address Register
    BAMR?   - Break Address Mask Register
    BBR?    - Break Bus Cycle Register (rules)

    BRCR    - Break Control Register (global rules)
*/

/* NOTE: UBC channel A registers. */

#define UBC_R_BARA      (REGISTER(int)      (_UBC_BASE + 0x00))
#define UBC_R_BAMRA     (REGISTER(char)     (_UBC_BASE + 0x04))
#define UBC_R_BBRA      (REGISTER(short)    (_UBC_BASE + 0x08))

/* NOTE: UBC channel B registers. */

#define UBC_R_BARB      (REGISTER(int)      (_UBC_BASE + 0x0C))
#define UBC_R_BAMRB     (REGISTER(char)     (_UBC_BASE + 0x10))
#define UBC_R_BBRB      (REGISTER(short)    (_UBC_BASE + 0x14))

/* NOTE: UBC global register. */

#define UBC_R_BRCR      (REGISTER(short)    (_UBC_BASE + 0x20))

/* NOTE: UBC register bitmasks. */

#define UBC_BAMR_NOASID     (1<<2)
#define UBC_BAMR_MASK_10    (1)
#define UBC_BBR_OPERAND     (1<<5)
#define UBC_BBR_INSTRUCT    (1<<4)
#define UBC_BBR_WRITE       (1<<3)
#define UBC_BBR_READ        (1<<2)
#define UBC_BBR_UNI         (UBC_BBR_OPERAND | UBC_BBR_INSTRUCT)
#define UBC_BBR_RW          (UBC_BBR_WRITE | UBC_BBR_READ)
#define UBC_BRCR_CMFA       (1<<15)
#define UBC_BRCR_CMFB       (1<<14)
#define UBC_BRCR_PCBA       (1<<10)
#define UBC_BRCR_PCBB       (1<<6)
#define UBC_BRCR_UBDE       (1)

/* NOTE: Exception/Interrupt registers. */

#define REG_EXPEVT          (REGISTER(uint32) (0xFF000024))
#define REG_INTEVT          (REGISTER(uint32) (0xFF000028))
#define REG_TRA             (REGISTER(uint32) (0xFF000020))

/* NOTE: Exception types from the lowlevel handler. */

#define EXP_TYPE_GEN        1
#define EXP_TYPE_CACHE      2
#define EXP_TYPE_INT        3

/* NOTE: SH4 exception codes. */

#define EXP_CODE_INT9       0x320
#define EXP_CODE_INT11      0x360
#define EXP_CODE_INT13      0x3A0
#define EXP_CODE_TRAP       0x160
#define EXP_CODE_UBC        0x1E0
#define EXP_CODE_RXI        0x720
#define EXP_CODE_BAD        0xFFF

/* NOTE: VBR vectors. */

#define VBR_GEN(tab)    ((void *) ((unsigned int) tab) + 0x100)
#define VBR_CACHE(tab)  ((void *) ((unsigned int) tab) + 0x400)
#define VBR_INT(tab)    ((void *) ((unsigned int) tab) + 0x600)

/* NOTE: System-level memory ranges. */

#define SYS_MEM_START       0x8C010000
#define SYS_MEM_END         0x8CFFFFFF

typedef struct
{
    uint32  fpscr;
    uint32  fpul;
    uint32  pr;
    uint32  mach;
    uint32  macl;

    uint32  vbr;
    uint32  gbr;
    uint32  sr;
    uint32  dbr;

    uint32  r7_bank;
    uint32  r6_bank;
    uint32  r5_bank;
    uint32  r4_bank;
    uint32  r3_bank;
    uint32  r2_bank;
    uint32  r1_bank;
    uint32  r0_bank;

    uint32  r14;
    uint32  r13;
    uint32  r12;
    uint32  r11;
    uint32  r10;
    uint32  r9;
    uint32  r8;
    uint32  r7;
    uint32  r6;
    uint32  r5;
    uint32  r4;
    uint32  r3;
    uint32  r2;
    uint32  r1;

    uint32  exception_type;
    uint32  r0;
} register_stack;

/* NOTE: External definitions. */

extern void *   dbr         (void);
extern void     dbr_set     (const void *set);

extern void *   vbr         (void);
extern void     vbr_set     (const void *set);

extern void *   spc         (void); 
extern void     spc_set     (const void *set);

extern uint32   r15         (void);
extern uint32   sr          (void);
extern uint32   fpscr       (void);
extern uint32   gbr         (void);

extern void     flush_cache (void);

extern void     ubc_wait    (void);

#endif
