/*  module.c

    $Id: module.c,v 1.13 2002/11/12 19:58:05 quad Exp $

DESCRIPTION

    Module callback core from the np-voot driver library.
    
    This one simply initializes the network library and passes on the
    callback handle for VOOT command handler.

*/

#include <vars.h>
#include <anim.h>
#include <lwip/net.h>
#include <timer.h>
#include <lwip/voot.h>

#include "scixb_emu.h"
#include "module.h"

static anim_render_chain_f      old_anim_chain;
static voot_packet_handler_f    old_voot_packet_handler;
static uint32                   version_count;

static void my_anim_chain (uint16 anim_code_a, uint16 anim_code_b)
{
    anim_printf_debug (0.0, 0.0, "Test module active. [%u]", version_count++);

    if (old_anim_chain)
        return old_anim_chain (anim_code_a, anim_code_b);
}

static bool my_voot_packet_handler (voot_packet *packet, void *ref)
{
    switch (packet->header.type)
    {
        case VOOT_PACKET_TYPE_COMMAND :
        {
            /* STAGE: Ensure there is actually a command. */

            if (!(packet->header.size))
                break;

            /* STAGE: Handle the version command. */

            if (packet->buffer[0] == VOOT_COMMAND_TYPE_VERSION)
                version_count++;

            break;
        }

        default :
            break;
    }

    return old_voot_packet_handler (packet, ref);
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

    /* STAGE: Initialize the networking drivers. */

    net_init ();
    voot_init ();

    old_voot_packet_handler = voot_add_packet_chain (my_voot_packet_handler);

    /* STAGE: Initialize the SCIXB emulation layer. */

    scixb_init ();

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

