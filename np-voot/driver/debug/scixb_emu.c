/*  scixb_emu.c

    $Id: scixb_emu.c,v 1.1 2002/08/04 05:48:05 quad Exp $

DESCRIPTION

    SCIXB RX and TX emulation layer. This doesn't emulate the chip, but it
    does trick SCIXB enough for reception.

*/

#include <vars.h>
#include <util.h>
#include <searchmem.h>
#include <ubc.h>
#include <exception.h>
#include <voot.h>
#include <serial.h>
#include <malloc.h>
#include <anim.h>

#include "scixb_emu.h"

#define USE_SERIAL

static uint16      *init_tx_root;
static const uint8  init_tx_key[]   = {
                                        0xe6, 0x2f, 0xd6, 0x2f, 0xc6, 0x2f,
                                        0xb6, 0x2f, 0xa6, 0x2f, 0x22, 0x4f,
                                        0xfc, 0x7f, 0xee, 0xbf, 0x40, 0x2f,
                                        0x08, 0x20, 0x1b, 0x89
                                      };
static uint16      *main_tx_root;
static const uint8  main_tx_key[]   = {
                                        0xe6, 0x2f, 0xd6, 0x2f, 0xc6, 0x2f,
                                        0xb6, 0x2f, 0xa6, 0x2f, 0x96, 0x2f,
                                        0x86, 0x2f, 0x22, 0x4f, 0xfc, 0x7f,
                                        0x0f, 0xdd, 0x11, 0xdc, 0x0b, 0x4d
                                      };

static const uint8  fifo_key[]      = {
                                        0x07, 0xd3, 0x2b, 0x43, 0x09, 0x00,
                                        0x07, 0xd3
                                      };

static bool                 inited;
static exception_handler_f  old_trap_handler;
static scixb_fifo_t        *voot_fifo;
static scixb_fifo_t         delay_fifo;

static anim_render_chain_f  old_anim_chain;

/*
    CREDIT: Reimplementation of the SCIXB interrupt handler.
    
    NOTE: Except we don't actually involve ourselves with the serial port.
*/

static void fifo_push (scixb_fifo_t *fifo, uint8 data)
{
    uint32  overflow_check;

    /*
        STAGE: Ensure that putting the next byte on won't overflow the FIFO.
    */

    overflow_check = (fifo->size - 1) & (fifo->end + 1);

    if (fifo->head == overflow_check)
    {
        /* STAGE: We've just overflown, so reset the FIFO and continue on... */

        fifo->status    = -1;
        fifo->end       = 0;
        fifo->head      = 0;
    }
    else
    {
        /* STAGE: Put the byte on the FIFO. */

        fifo->pointer[fifo->end++] = data;

        fifo->end &= (fifo->size - 1);
    }
}

static int32 fifo_pull (scixb_fifo_t *fifo)
{
    int32   retchar;

    retchar = -1;

    /* STAGE: Is there something in the FIFO? */

    if (fifo->head != fifo->end)
    {
        retchar = fifo->pointer[fifo->head++];

        fifo->head &= (fifo->size - 1);
    }

    return retchar;
}

static void my_anim_chain (uint16 anim_mode_a, uint16 anim_mode_b)
{
    /* STAGE: Handle all cached bytes. */

    if (delay_fifo.pointer)
    {
        int32   out_data;
        uint8   out_data_buffer[32];
        uint32  out_data_buffer_index;

        out_data_buffer_index = 0;

        while (((out_data = fifo_pull (&delay_fifo)) >= 0) && (out_data_buffer_index < sizeof (out_data_buffer)))
            out_data_buffer[out_data_buffer_index++] = out_data;

        if (out_data_buffer_index)
        {
            voot_send_packet (VOOT_PACKET_TYPE_DATA, out_data_buffer, out_data_buffer_index);

#ifdef USE_SERIAL
            serial_write_buffer (out_data_buffer, out_data_buffer_index);
#endif
        }
    }

    if (old_anim_chain)
        return old_anim_chain (anim_mode_a, anim_mode_b);
}

static void tx_handler (uint8 in_data, bool main_tx)
{
    if (main_tx && delay_fifo.pointer)
    {
        fifo_push (&delay_fifo, in_data);
    }
    else
    {
        voot_send_packet (VOOT_PACKET_TYPE_DATA, &in_data, sizeof (in_data));

#ifdef USE_SERIAL
        serial_write_char (in_data);
#endif
    }
}

static void* trap_handler (register_stack *stack, void *current_vector)
{
    /* STAGE: Handle the channels we know about. */

    switch (ubc_trap_number ())
    {
        case TRAP_CODE_SCIXB_TXI :
            tx_handler (stack->r4, FALSE);
            spc_set ((void *) stack->pr);
            break;

        case TRAP_CODE_SCIXB_TXM :
            tx_handler (stack->r4, TRUE);
            spc_set ((void *) stack->pr);
            break;
    
        default :
            break;
    }

    /*
        STAGE: If there is someone higher on the vector, let them know about
        the trap.
    */

    if (old_trap_handler)
        return old_trap_handler (stack, current_vector);
    else
        return current_vector;
}


void scixb_init (void)
{
    uint16  init_trap;
    uint16  main_trap;

    /*
        STAGE: Set the SCIF into loopback mode, so we don't have to worry
        about the serial port weirding up.
    */

#ifndef USE_SERIAL
    *SCIF_R_FC |= SCIF_FC_LOOP;
#endif

    /* STAGE: Generate our traps for comparison testing... */

    init_trap = ubc_generate_trap (TRAP_CODE_SCIXB_TXI);
    main_trap = ubc_generate_trap (TRAP_CODE_SCIXB_TXM);

    /* STAGE: Locate both TX functions. */

    if (!init_tx_root || memcmp (init_tx_root, &init_trap, sizeof (init_trap)))
        init_tx_root = search_gamemem (init_tx_key, sizeof (init_tx_key));

    if (!main_tx_root || memcmp (main_tx_root, &main_trap, sizeof (main_trap)))
        main_tx_root = search_gamemem (main_tx_key, sizeof (main_tx_key));

    /*
        STAGE: If we haven't already, configure the exception table to pass
        us our traps.
    */

    if (!inited)
    {
        exception_table_entry   new_entry;

        new_entry.type      = EXP_TYPE_GEN;
        new_entry.code      = EXP_CODE_TRAP;
        new_entry.handler   = trap_handler;

        inited = exception_add_handler (&new_entry, &old_trap_handler);

        anim_add_render_chain (my_anim_chain, &old_anim_chain);
    }

    if (init_tx_root && main_tx_root && inited)
    {
        /* STAGE: Locate the SCIXB FIFO. */

        voot_fifo = (scixb_fifo_t *) *( ( (uint32 *) search_gamemem (fifo_key, sizeof (fifo_key)) ) - 1 );

        /* STAGE: (Re-)Initialize the delay FIFO. */

        if (!delay_fifo.pointer)
        {
            delay_fifo.size     = voot_fifo->size;
            delay_fifo.pointer  = malloc (delay_fifo.size);
        }

        delay_fifo.head = delay_fifo.end = delay_fifo.status = 0;

        /* STAGE: Write the trappoints out... */

        init_tx_root[0] = init_trap;
        main_tx_root[0] = main_trap;
    }
}
