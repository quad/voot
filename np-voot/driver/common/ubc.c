/*  ubc.c

    $Id: ubc.c,v 1.1 2002/06/23 03:22:52 quad Exp $

DESCRIPTION

    Routines for configuring and handling the UBC.

*/

#include "vars.h"
#include "system.h"

#include "ubc.h"

void ubc_init (void)
{
    /* STAGE: Clear both UBC channels. */

    *UBC_R_BBRA     = *UBC_R_BBRB   = 0;
    *UBC_R_BAMRA    = *UBC_R_BAMRB  = UBC_BAMR_NOASID;

    /* STAGE: Initialize the global UBC configuration. */

    *UBC_R_BRCR     = UBC_BRCR_UBDE | UBC_BRCR_PCBA | UBC_BRCR_PCBB;
}

void ubc_configure_channel (ubc_channel channel, uint32 breakpoint, uint16 options)
{
    switch (channel)
    {
        case UBC_CHANNEL_A :
        {
            *UBC_R_BARA = breakpoint;
            *UBC_R_BBRA = options;

            break;
        }

        case UBC_CHANNEL_B :
        {
            *UBC_R_BARB = breakpoint;
            *UBC_R_BBRB = options;

            break;
        }

        /*
            STAGE: If it isn't one of the two channels, we really can't do
            anything anyway...
        */

        default :
            return;
    }

    ubc_wait ();
}

void ubc_clear_channel (ubc_channel channel)
{
    switch (channel)
    {
        case UBC_CHANNEL_A :
        {
            /* STAGE: Clear the UBC channel. */

            *UBC_R_BBRA = 0;

            /* STAGE: Clear the break bit. */

            *UBC_R_BRCR &= ~(UBC_BRCR_CMFA);

            break;
        }

        case UBC_CHANNEL_B :
        {
            /* STAGE: Clear the UBC channel. */

            *UBC_R_BBRB = 0;

            /* STAGE: Clear the break bit. */

            *UBC_R_BRCR &= ~(UBC_BRCR_CMFB);

            break;
        }

        /*
            STAGE: If it isn't one of the two channels, we really can't do
            anything anyway...
        */

        default :
            return;
    }

    ubc_wait ();
}

uint16 ubc_generate_trap (uint8 trap_code)
{
    /*
        NOTE: The trap instruction is formed as such:
        
        11 00 00 11 ii ii ii ii
    */

    return (0xC300) | trap_code;
}
