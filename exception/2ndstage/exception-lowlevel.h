#ifndef __EXCEPTION_LOWLEVEL_H__
#define __EXCEPTION_LOWLEVEL_H__

#include "vars.h"

/* The UBC Registers */
#define _UBC_BASE       (0xFF200000)
#define _UBC_ASID_BASE  (0xFF000000)

/*
    BAR?    - Break Address Register
    BAMR?   - Break Address Mask Register
    BBR?    - Break Bus Cycle Register (rules)

    BRCR    - Break Control Register (global rules)
*/

/* Channel A */
#define UBC_R_BARA      (REGISTER(int)      (_UBC_BASE + 0x00))
#define UBC_R_BAMRA     (REGISTER(char)     (_UBC_BASE + 0x04))
#define UBC_R_BBRA      (REGISTER(short)    (_UBC_BASE + 0x08))

/* Channel B */
#define UBC_R_BARB      (REGISTER(int)      (_UBC_BASE + 0x0C))
#define UBC_R_BAMRB     (REGISTER(char)     (_UBC_BASE + 0x10))
#define UBC_R_BBRB      (REGISTER(short)    (_UBC_BASE + 0x14))

/* UBC Global */
#define UBC_R_BRCR      (REGISTER(short)    (_UBC_BASE + 0x20))

/* UBC Bitmasks */
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

extern uint8 general_sub_handler[];
extern uint8 general_sub_handler_base[];
extern uint8 general_sub_handler_end[];

extern uint8 cache_sub_handler[];
extern uint8 cache_sub_handler_base[];
extern uint8 cache_sub_handler_end[];

extern uint8 interrupt_sub_handler[];
extern uint8 interrupt_sub_handler_base[];
extern uint8 interrupt_sub_handler_end[];

extern void exception_handler_lowlevel(void);
extern void my_exception_finish(void);
extern void ubc_wait(void);

#endif
