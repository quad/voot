/*  module.c

    $Id: module.c,v 1.12 2002/11/07 02:24:05 quad Exp $

DESCRIPTION

    Module callback core from the np-voot driver library.
    
    This one simply initializes the network library and passes on the
    callback handle for VOOT command handler.

*/

#include <vars.h>
#include <anim.h>

#include "scixb_emu.h"
#include "module.h"

static anim_render_chain_f      old_anim_chain;

static void my_anim_chain (uint16 anim_code_a, uint16 anim_code_b)
{
    anim_printf_debug (0.0, 0.0, "Test module active.");

    if (old_anim_chain)
        return old_anim_chain (anim_code_a, anim_code_b);
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
    /* STAGE: Initialize our debugging OSD. */

    anim_init ();

    anim_add_render_chain (my_anim_chain, &old_anim_chain);

#ifdef SCIXB
    /* STAGE: Initialize the serial buffer. */

    scixb_init ();
#endif
}

void module_bios_vector (void)
{
    /*
        NOTE: We don't really need to be anymore paranoid than the main
        driver core.
    */
}
