/*  exception.h 

    $Id: exception.h,v 1.2 2002/06/12 09:33:51 quad Exp $

*/

#ifndef __COMMON_EXCEPTION_H__
#define __COMMON_EXCEPTION_H__

#include "vars.h"
#include "system.h"

/* NOTE: The UBC Registers. */

#define _UBC_BASE       (0xff200000)
#define _UBC_ASID_BASE  (0xff000000)

/*
    NOTE: Abbreviation expansion...

    BAR?    - Break Address Register
    BAMR?   - Break Address Mask Register
    BBR?    - Break Bus Cycle Register (rules)

    BRCR    - Break Control Register (global rules)
*/

/* NOTE: Channel A */

#define UBC_R_BARA      (REGISTER(int)      (_UBC_BASE + 0x00))
#define UBC_R_BAMRA     (REGISTER(char)     (_UBC_BASE + 0x04))
#define UBC_R_BBRA      (REGISTER(short)    (_UBC_BASE + 0x08))

/* NOTE: Channel B */

#define UBC_R_BARB      (REGISTER(int)      (_UBC_BASE + 0x0C))
#define UBC_R_BAMRB     (REGISTER(char)     (_UBC_BASE + 0x10))
#define UBC_R_BBRB      (REGISTER(short)    (_UBC_BASE + 0x14))

/* NOTE: UBC Global */

#define UBC_R_BRCR      (REGISTER(short)    (_UBC_BASE + 0x20))

/* NOTE: UBC Bitmasks */

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

/* NOTE: VBR exception vectors */

#define VBR_GEN(tab)    ((void *) ((unsigned int) tab) + 0x100)
#define VBR_CACHE(tab)  ((void *) ((unsigned int) tab) + 0x400)
#define VBR_INT(tab)    ((void *) ((unsigned int) tab) + 0x600)

#define EXP_TABLE_SIZE  7

typedef struct
{
    uint32  type;
    uint32  code;
    void    *(*handler)(register_stack *, void *);
} exception_table_entry;

typedef struct
{
    /* NOTE: Exception counters */

    uint32      general_exception_count;
    uint32      cache_exception_count;
    uint32      interrupt_exception_count;
    uint32      ubc_exception_count;
    uint32      odd_exception_count;

    /* NOTE: Function hooks for various interrupts */

    exception_table_entry   table[EXP_TABLE_SIZE];

    /* NOTE: Private status information */

    bool        vbr_switched;
} exception_table;

typedef enum
{
    UBC_CHANNEL_A,
    UBC_CHANNEL_B
} ubc_channel;

/* NOTE: Module Declarations */

void    ubc_init                (void);
void    ubc_configure_channel   (ubc_channel channel, uint32 breakpoint, uint16 options);
void    ubc_clear_channel       (ubc_channel channel);

uint32  add_exception_handler   (const exception_table_entry *new_entry);
void *  exception_handler       (register_stack *stack);

#endif
