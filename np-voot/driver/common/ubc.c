/*  ubc.c

    $Id: ubc.c,v 1.3 2002/07/06 14:18:15 quad Exp $

DESCRIPTION

    Routines for configuring and handling the UBC.

*/

#include "vars.h"
#include "system.h"
#include "exception.h"

#include "ubc.h"

static bool                 inited;
static exception_handler_f  old_ubc_handler;

static void* ubc_handler (register_stack *stack, void *current_vector)
{
    /* STAGE: Give anyone below us a chance at the breakpoint. */

    if (old_ubc_handler)
        current_vector = old_ubc_handler (stack, current_vector);

    /*
        STAGE: Since we're, supposedly, the last handler running.. clear all
        UBC exceptions.
    */

    if (ubc_is_channel_break (UBC_CHANNEL_A))
        ubc_clear_break (UBC_CHANNEL_A);

    if (ubc_is_channel_break (UBC_CHANNEL_B))
        ubc_clear_break (UBC_CHANNEL_B);

    return current_vector;
}

void ubc_init (void)
{
    exception_table_entry   new_entry;

    /* STAGE: Clear both UBC channels. */

    *UBC_R_BBRA     = *UBC_R_BBRB   = 0;
    *UBC_R_BAMRA    = *UBC_R_BAMRB  = UBC_BAMR_NOASID;

    /* STAGE: Initialize the global UBC configuration. */

    *UBC_R_BRCR     = UBC_BRCR_UBDE | UBC_BRCR_PCBA | UBC_BRCR_PCBB;

    /* STAGE: Ensure interrupts are being directed properly. */

    dbr_set (ubc_handler_lowlevel);

    /* STAGE: Don't allow multiple hooks on the exception. */

    if (inited)
        return;

    /* STAGE: Install the UBC clearing handler. */

    new_entry.type      = EXP_TYPE_GEN;
    new_entry.code      = EXP_CODE_UBC;
    new_entry.handler   = ubc_handler;

    inited = exception_add_handler (&new_entry, &old_ubc_handler);
}

bool ubc_configure_channel (ubc_channel channel, uint32 breakpoint, uint16 options)
{
    /* STAGE: Ensure the UBC has been initialized. */

    if (!inited)
        return FALSE;

    /*
        STAGE: Configure the appropriate channel with the specified
        breakpoint.
    */

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
            return FALSE;
    }

    ubc_wait ();

    return TRUE;
}

void ubc_clear_channel (ubc_channel channel)
{
    /* STAGE: Clear the appropriate channel's options. */

    switch (channel)
    {
        case UBC_CHANNEL_A :
        {
            *UBC_R_BBRA = 0;

            ubc_clear_break (channel);

            break;
        }

        case UBC_CHANNEL_B :
        {
            *UBC_R_BBRB = 0;

            ubc_clear_break (channel);

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

void ubc_clear_break (ubc_channel channel)
{
    /* STAGE: Clear the appropriate channel's break bit. */

    switch (channel)
    {
        case UBC_CHANNEL_A :
            *UBC_R_BRCR &= ~(UBC_BRCR_CMFA);
            break;

        case UBC_CHANNEL_B :
            *UBC_R_BRCR &= ~(UBC_BRCR_CMFB);
            break;

        /*
            STAGE: If it isn't one of the two channels, we really can't do
            anything anyway...
        */
    
        default :
            return;
    }
}

bool ubc_is_channel_break (ubc_channel channel)
{
    switch (channel)
    {
        case UBC_CHANNEL_A :
            return !!(*UBC_R_BRCR & UBC_BRCR_CMFA);
            break;

        case UBC_CHANNEL_B :
            return !!(*UBC_R_BRCR & UBC_BRCR_CMFB);
            break;

        /*
            STAGE: If we don't know about the channel, we can't respond with
            anything coherent.
        */

        default :
            return FALSE;
    }
}

uint16 ubc_generate_trap (uint8 trap_code)
{
    /*
        NOTE: The trap instruction is formed as such:
        
        11 00 00 11 ii ii ii ii
    */

    return (0xC300) | trap_code;
}

uint8 ubc_trap_number (void)
{
    /* STAGE: Obtain the actual TRAPA argument. */

    return ((*REG_TRA >> 2) & 0xff);
}
