/*  module.c

    $Id: module.c,v 1.7 2002/07/06 14:18:16 quad Exp $

DESCRIPTION

    Module callback core from the np-voot driver library.
    
    This one simply initializes the network library and passes on the
    callback handle for VOOT command handler.

*/

#include <vars.h>
#include <anim.h>

#include <assert.h>

#include "scixb_emu.h"
#include "module.h"

static anim_render_chain_f  old_anim_chain;

static void my_anim_chain (uint16 anim_code_a, uint16 anim_code_b)
{
    anim_printf_debug (0.0, 0.0, "Test module active.");
}

/*
    NOTE: Module callback functions.
    
    These are called by the core library to give linkages back to the
    module.
*/

void module_initialize (void)
{
    /* NOTE: We don't need to initialize anything. */
}

void module_configure (void)
{
    /*
        NOTE: The following are emblem breakpoints in game54.bin

        memcpy'ing in of the emblem buffer.
        ubc_configure_channel (UBC_CHANNEL_A, (uint32) 0x8c0f9c30, UBC_BBR_WRITE | UBC_BBR_OPERAND);

        main emblem (possibly replay) loader function.
        ubc_configure_channel (UBC_CHANNEL_A, 0x8c018f4c, UBC_BBR_READ | UBC_BBR_INSTRUCT);

    */

    anim_init ();

    anim_add_render_chain (my_anim_chain, &old_anim_chain);
}

void module_bios_vector (void)
{
    /*
        NOTE: We don't really need to be anymore paranoid than the main
        driver core.
    */
}
