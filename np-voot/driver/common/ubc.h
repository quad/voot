/*  ubc.h

    $Id: ubc.h,v 1.3 2002/06/30 09:15:06 quad Exp $

*/

#ifndef __COMMON_UBC_H__
#define __COMMON_UBC_H__

#include "system.h"

/* NOTE: UBC register bitmasks. */

#define UBC_BAMR_NOASID     (1<<2)
#define UBC_BAMR_MASK_10    (1)
#define UBC_BAMR_MASK_12    (1<<1)
#define UBC_BAMR_MASK_16    (1<<3)
#define UBC_BAMR_MASK_20    (UBC_BAMR_MASK_16 | UBC_BAMR_MASK_10)
#define UBC_BAMR_MASK_ALL   (UBC_BAMR_MASK_10 | UBC_BAMR_MASK_12)

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

/* NOTE: Predefined traps. (reserved by the common library) */

#define TRAP_CODE_ANIM      (0xFF)
#define TRAP_CODE_SCIXB_TXI (0xFE)
#define TRAP_CODE_SCIXB_TXM (0xFD)

typedef enum
{
    UBC_CHANNEL_A,
    UBC_CHANNEL_B
} ubc_channel;

/* NOTE: Module definitions. */

void    ubc_init                (void);
void    ubc_configure_channel   (ubc_channel channel, uint32 breakpoint, uint16 options);
void    ubc_clear_channel       (ubc_channel channel);
void    ubc_clear_break         (ubc_channel channel);
bool    ubc_is_channel_break    (ubc_channel channel);
uint16  ubc_generate_trap       (uint8 trap_code);
uint8   ubc_trap_number         (void);

#endif
