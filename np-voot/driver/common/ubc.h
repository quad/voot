/*  ubc.h

    $Id: ubc.h,v 1.1 2002/06/23 03:22:52 quad Exp $

*/

#ifndef __COMMON_UBC_H__
#define __COMMON_UBC_H__

typedef enum
{
    UBC_CHANNEL_A,
    UBC_CHANNEL_B
} ubc_channel;

/* NOTE: Module definitions. */

void    ubc_init                (void);
void    ubc_configure_channel   (ubc_channel channel, uint32 breakpoint, uint16 options);
void    ubc_clear_channel       (ubc_channel channel);
uint16  ubc_generate_trap       (uint8 trap_code);

#endif
