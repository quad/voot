/*  module.c

    $Id: module.c,v 1.18 2002/12/17 11:55:01 quad Exp $

DESCRIPTION

    Module callback core from the np-voot driver library.
    
    This one simply initializes the network library and passes on the
    callback handle for VOOT command handler.

NOTES

    8c0397d8 (byte) - actual TX
    8c018dd8 (init, data, data_size) = {0 continue, 3 syncing, 1 success} - actual TX function in INIT
    8c018c06 (init, ?data, ?data_size) = {0 abort, 1 continue, 2 success} - actual INIT function (8c022072 JP)

    8c093ca4 - voot RX fifo
    8c03990c (void) = {byte} - uni RX routine

    8c018ab6 - second stage init RX handler
    8c018dd8 - first stage init RX/TX sync handler

*/

#include <vars.h>
#include <anim.h>

#include <gamedata.h>
#include <dumpio.h>

#include <lwip/net.h>
#include <lwip/voot.h>

#include "scixb_emu.h"
#include "module.h"

static anim_render_chain_f      old_anim_chain;
static uint32                   reconf_count;

static void my_anim_chain (uint16 anim_code_a, uint16 anim_code_b)
{
    anim_printf_debug (0.0, 0.0, "Test module active. [reconf %u]", reconf_count);

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

    /* STAGE: Initialize the networking drivers. */

    net_init ();
    voot_init ();
    dump_init ();

    /* STAGE: Initialize the SCIXB emulation layer. */

    //scixb_init ();
}

void module_reconfigure (void)
{
    net_init ();
    voot_init ();
    dump_init ();

    reconf_count++;
}

void module_bios_vector (void)
{
    /*
        NOTE: We don't really need to be anymore paranoid than the main
        driver core.
    */
}
