/*  module.c

    $Id: module.c,v 1.10 2002/11/04 18:38:26 quad Exp $

DESCRIPTION

    Module callback core from the np-voot driver library.
    
    This one simply initializes the network library and passes on the
    callback handle for VOOT command handler.

*/

#include <vars.h>
#include <ether.h>
#include <anim.h>
#include <dumpio.h>
#include <gamedata.h>
#include <searchmem.h>
#include <exception.h>
#include <ubc.h>
#include <util.h>

#include "scixb_emu.h"
#include "net-lwip.h"
#include "module.h"

static anim_render_chain_f      old_anim_chain;
static voot_packet_handler_f    old_voot_chain;
static exception_handler_f      old_ubc_handler;

static void my_anim_chain (uint16 anim_code_a, uint16 anim_code_b)
{
    anim_printf_debug (0.0, 0.0, "Test module active. [%x, %x, %x]", *((uint8 *) 0x8ccf61c0), *((uint8 *) 0x8c260008), GAMEDATA_OPT->controller_req);

    if (old_anim_chain)
        return old_anim_chain (anim_code_a, anim_code_b);
}

static bool my_voot_chain (voot_packet *packet)
{
    switch (packet->header.type)
    {
        case VOOT_PACKET_TYPE_COMMAND :
        {
            /* STAGE: Ensure there is actually a command. */

            if (!(packet->header.size))
                break;

            switch (packet->buffer[0])
            {
                case VOOT_COMMAND_TYPE_DEBUG :
                {
                    char name[] = "Scott";

                    strncpy ((char *) 0x8c2e7f40, name, 16);
                    voot_debug ("%s copied into P1 name data", name);

                    GAMEDATA_OPT->controller_req = 0x4;
                    voot_debug ("controller_req set to %x", GAMEDATA_OPT->controller_req);

                    break;
                }
            }

            break;
        }
    }

    return old_voot_chain (packet);
}

static void* my_ubc_handler (register_stack *stack, void *current_vector)
{
    if (ubc_is_channel_break (UBC_CHANNEL_B))
    {
        if (stack->pr == 0x8c39d39a)
            voot_debug ("UBC_CHANNEL_B (%x, %x, %x)", stack->r4, stack->r5, stack->r6);
    }

    if (old_ubc_handler)
        return old_ubc_handler (stack, current_vector);
    else
        return current_vector;
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
    exception_table_entry   new_exp;

    /*
        NOTE: The following are emblem breakpoints in game54.bin

        memcpy'ing in of the emblem buffer.
        ubc_configure_channel (UBC_CHANNEL_A, (uint32) 0x8c0f9c30, UBC_BBR_WRITE | UBC_BBR_OPERAND);

        main emblem (possibly replay) loader function.
        ubc_configure_channel (UBC_CHANNEL_A, 0x8c018f4c, UBC_BBR_READ | UBC_BBR_INSTRUCT);

        possible replay customization configuration function
        ubc_configure_channel (UBC_CHANNEL_A, 0x8c39d1bc, UBC_BBR_READ | UBC_BBR_INSTRUCT);

    */

    /* STAGE: Ensure we're ready for the UBC exception. */

    new_exp.type    = EXP_TYPE_GEN;
    new_exp.code    = EXP_CODE_UBC;
    new_exp.handler = my_ubc_handler;

    if (exception_add_handler (&new_exp, &old_ubc_handler))
        ubc_configure_channel (UBC_CHANNEL_B, 0x8c262170, UBC_BBR_READ | UBC_BBR_OPERAND);

    if (ether_init ())
    {
        dump_init ();
        old_voot_chain = voot_add_packet_chain (my_voot_chain);
    }

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
