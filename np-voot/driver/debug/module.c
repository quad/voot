/*  module.c

    $Id: module.c,v 1.6 2002/06/23 03:22:52 quad Exp $

DESCRIPTION

    Module callback core from the np-voot driver library.
    
    This one simply initializes the network library and passes on the
    callback handle for VOOT command handler.

*/

#include <vars.h>
#include <ubc.h>
#include <exception.h>
#include <exception-lowlevel.h>
#include <searchmem.h>
#include <gamedata.h>
#include <printf.h>
#include <rtl8139c.h>
#include <dumpio.h>
#include <voot.h>
#include <serial.h>

#include "module.h"

static uint16          *anim_mode_a     = (uint16 *)    0x8ccf0228;
static uint16          *anim_mode_b     = (uint16 *)    0x8ccf022a;

static const uint8  osd_func_key[]      = {
                                            0xe6, 0x2f, 0xd6, 0x2f, 0xc6,
                                            0x2f, 0xb6, 0x2f, 0xa6, 0x2f,
                                            0x96, 0x2f, 0x86, 0x2f, 0xfb,
                                            0xff, 0x22, 0x4f, 0xfc, 0x7f,
                                            0x43, 0x6d, 0x00, 0xe4, 0x43,
                                            0x6b
                                          };

static voot_packet_handler_f    old_voot_packet_handler;

bool debug_packet_handler (voot_packet *packet)
{
    switch (packet->header.type)
    {
        case VOOT_PACKET_TYPE_COMMAND :
        {
            /* STAGE: Ensure there is actually a command. */

            if (!(packet->header.size))
                break;

            /* STAGE: Handle the version command. */

            if (packet->buffer[0] == VOOT_COMMAND_TYPE_DEBUG)
            {
                voot_debug ("anim_mode_a [%u] anim_mode_b [%u]", *anim_mode_a, *anim_mode_b);
            }

            break;
        }
    }

    return old_voot_packet_handler (packet);
}

void module_initialize (void)
{
    /* NOTE: We don't need to initialize anything special. */
}

void module_configure (void)
{
    exception_table_entry   new;

    /* STAGE: Configure the UBC channels for the animation and level select breakpoints. */

    ubc_configure_channel (UBC_CHANNEL_A, (uint32) anim_mode_b, UBC_BBR_WRITE | UBC_BBR_OPERAND);

    ubc_configure_channel (UBC_CHANNEL_B, (uint32) SCIF_R_RD, UBC_BBR_READ | UBC_BBR_OPERAND);

    /*
        NOTE: The following are the transmit functions for VOOT.

        Initialization TX line.

        ubc_configure_channel (UBC_CHANNEL_B, (uint32) 0x8c0397d8, UBC_BBR_READ | UBC_BBR_INSTRUCT);

        Main TX line.

        ubc_configure_channel (UBC_CHANNEL_B, (uint32) 0x8c039844, UBC_BBR_READ | UBC_BBR_INSTRUCT);

    */

    /* STAGE: Add our handler to the queue. */

    new.type    = EXP_TYPE_GEN;
    new.code    = EXP_CODE_UBC;
    new.handler = debug_handler;

    exception_add_handler (&new);

    /* STAGE: Configure the trap handler on both IRQ lines. */

    new.code    = EXP_CODE_TRAP;
    new.handler = trap_handler;

    exception_add_handler (&new);

    *((uint16 *) 0x8c0397d8) = ubc_generate_trap (0x01);
    *((uint16 *) 0x8c039844) = ubc_generate_trap (0x02);

    /* STAGE: Configure the networking. */

    if (pci_detect ())
    {
        if (pci_bb_init ())
        {
            if (rtl_init ())
            {
                /* STAGE: Add another VOOT protocol handler. */

                old_voot_packet_handler = voot_add_packet_chain (debug_packet_handler);
            }
        }
    }
}

void module_bios_vector (void)
{
    /*
        NOTE: We don't really need to be anymore paranoid than the main
        driver core.
    */
}

static void* my_anim_handler (register_stack *stack, void *current_vector)
{
    gamedata_enable_debug ();

    return current_vector;
}

static void* my_debug_handler (register_stack *stack, void *current_vector)
{
    if (spc () == (void *) 0x8c039b58)
        voot_debug ("RX [%x] '%c'", stack->r4);
    else
        voot_debug ("RX from spc() [%x]", spc ());

    return current_vector;
}

void* debug_handler (register_stack *stack, void *current_vector)
{
    if (*UBC_R_BRCR & UBC_BRCR_CMFA)
    {
        /* STAGE: Be sure to clear the proper bit. */

        *UBC_R_BRCR &= ~(UBC_BRCR_CMFA);

        /* STAGE: Pass control to the actual code base. */

        current_vector = my_anim_handler (stack, current_vector);
    }

    if (*UBC_R_BRCR & UBC_BRCR_CMFB)
    {
        /* STAGE: Be sure to clear the proper bit. */

        *UBC_R_BRCR &= ~(UBC_BRCR_CMFB);

        /* STAGE: Pass control to the actual code base. */

        current_vector = my_debug_handler (stack, current_vector);
    }

    return current_vector;
}

void* trap_handler (register_stack *stack, void *current_vector)
{
    uint8   trap_number;
    char    data;

    /* STAGE: Obtain the actual TRAPA argument. */

    trap_number = ((*REG_TRA >> 2) & 0xff);

    /* STAGE: Obtain the data to be transmitted. */

    data = stack->r4;

    /* STAGE: Actually retransmit the appropriate data. */

    serial_write_char (data);

    /* STAGE: Perform a pseudo RTS instruction. */

    spc_set ((void *) stack->pr);

    return my_exception_finish;
}
