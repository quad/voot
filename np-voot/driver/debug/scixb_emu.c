/*  scixb_emu.c

    $Id: scixb_emu.c,v 1.4 2002/11/14 20:56:08 quad Exp $

DESCRIPTION

    SCIXB RX and TX emulation layer. This doesn't emulate the chip, but it
    does trick SCIXB enough for reception.

*/

#include <vars.h>
#include <util.h>
#include <searchmem.h>
#include <ubc.h>
#include <exception.h>
#include <serial.h>
#include <malloc.h>
#include <anim.h>

#include <lwip/voot.h>

#include "scixb_emu.h"

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
static uint16      *init_link_root;
static const uint8  init_link_key[] = {
                                        0xe6, 0x2f, 0x48, 0x24, 0xd6, 0x2f,
                                        0xc6, 0x2f, 0xb6, 0x2f
                                      };
extern void         init_link_continue  (void);
asm ("
_init_link_continue:
init_link_emulate_start:
    mov.l   r14, @-r15
init_link_return:
    mov.l   _init_link_root, r0
    add     #2, r0
    jmp     @r0
    nop
");

static const uint8  fifo_key[]      = {
                                        0x07, 0xd3, 0x2b, 0x43, 0x09, 0x00,
                                        0x07, 0xd3
                                      };

static bool                     inited;
static bool                     init_link_lock;
static uint32                   init_link_lock_time;
static exception_handler_f      old_trap_handler;
#ifdef DEBUG_RX
static exception_handler_f      old_rxi_handler;
#endif
static scixb_fifo_t            *voot_fifo;
static scixb_fifo_t             delay_fifo;

static anim_render_chain_f      old_anim_chain;
static voot_packet_handler_f    old_voot_packet_handler;

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

static bool my_voot_packet_handler (voot_packet *packet, void *ref)
{
    switch (packet->header.type)
    {
        case VOOT_PACKET_TYPE_DATA :
        {
            if (voot_fifo)
            {
                uint32  index;

                for (index = 0; index < (packet->header.size - 1); index++)
                    fifo_push (voot_fifo, packet->buffer[index]);

                if (init_link_lock)
                    init_link_lock = FALSE;
            }

            break;
        }
    }

    return old_voot_packet_handler (packet, ref);
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

#ifdef SERIAL_PASSTHROUGH
            serial_write_buffer (out_data_buffer, out_data_buffer_index);
#endif
        }
    }

    if (init_link_lock && init_link_lock_time >= time ())
        anim_printf_debug (0.0, 15.0, "Link init. timeout remaining: %us", init_link_lock_time - time ());

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

        init_link_lock      = TRUE;
        init_link_lock_time = 0;
#ifdef SERIAL_PASSTHROUGH
        serial_write_char (in_data);
#endif
    }
}

#ifdef DEBUG_RX
static void* rxi_handler (register_stack *stack, void *current_vector)
{
    if (init_link_lock)
        init_link_lock = FALSE;

    if (old_rxi_handler)
        return old_rxi_handler (stack, current_vector);
    else
        return current_vector;
}
#endif

static void* trap_handler (register_stack *stack, void *current_vector)
{
    /* STAGE: Handle the channels we know about. */

    switch (ubc_trap_number ())
    {
        case TRAP_CODE_SCIXB_TXI :
            tx_handler (stack->r4, FALSE);
            stack->spc = stack->pr;
            break;

        case TRAP_CODE_SCIXB_TXM :
            tx_handler (stack->r4, TRUE);
            stack->spc = stack->pr;
            break;

        case TRAP_CODE_SCIXB_ILK :
        {
            switch (stack->r4)
            {
                case 0x0 :
                    if (init_link_lock && time () < init_link_lock_time)
                    {
                        stack->r0   = 1;
                        stack->spc  = stack->pr;
                        voot_debug ("init_link locked! [%u remaining]", init_link_lock_time - time ());
                    }
                    else
                    {
                        init_link_lock = FALSE;

                        if (!init_link_lock_time)
                        {
                            init_link_lock      = TRUE;
                            init_link_lock_time = time () + 10;
                        }

                        stack->spc = (uint32) init_link_continue;
                    }
                    break;

                case 0x1 :
                    init_link_lock = FALSE;
                    stack->spc = (uint32) init_link_continue;
                    break;
            }

            break;
        }

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
    uint16  link_trap;

    /*
        STAGE: Set the SCIF into loopback mode, so we don't have to worry
        about the serial port weirding up.
    */

    /* STAGE: Generate our traps for comparison testing... */

    init_trap = ubc_generate_trap (TRAP_CODE_SCIXB_TXI);
    main_trap = ubc_generate_trap (TRAP_CODE_SCIXB_TXM);
    link_trap = ubc_generate_trap (TRAP_CODE_SCIXB_ILK);

    /* STAGE: Locate both TX functions. */

    if (!init_tx_root || memcmp (init_tx_root, &init_trap, sizeof (init_trap)))
        init_tx_root = search_gamemem (init_tx_key, sizeof (init_tx_key));

    if (!main_tx_root || memcmp (main_tx_root, &main_trap, sizeof (main_trap)))
        main_tx_root = search_gamemem (main_tx_key, sizeof (main_tx_key));

    /* STAGE: Locate the link initalization function. */

    if (!init_link_root || memcmp (init_link_root, &link_trap, sizeof (link_trap)))
        init_link_root = search_gamemem (init_link_key, sizeof (init_link_key));

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

#ifdef DEBUG_RX
        new_entry.type      = EXP_TYPE_INT;
        new_entry.code      = EXP_CODE_RXI;
        new_entry.handler   = rxi_handler;

        exception_add_handler (&new_entry, &old_rxi_handler);
#endif

        anim_add_render_chain (my_anim_chain, &old_anim_chain);
    }

    if (init_tx_root && main_tx_root && init_link_root && inited)
    {
        /* STAGE: Locate the SCIXB FIFO. */

        voot_fifo = (scixb_fifo_t *) *( ( (uint32 *) search_gamemem (fifo_key, sizeof (fifo_key)) ) - 1 );

        /* STAGE: Add ourselves to the VOOT packet handling chain. */

        old_voot_packet_handler = voot_add_packet_chain (my_voot_packet_handler);

        /* STAGE: (Re-)Initialize the delay FIFO. */

        if (!delay_fifo.pointer)
        {
            delay_fifo.size     = voot_fifo->size;
            delay_fifo.pointer  = malloc (delay_fifo.size);
        }

        delay_fifo.head = delay_fifo.end = delay_fifo.status = 0;

        /* STAGE: Write the trappoints out... */

        init_tx_root[0]     = init_trap;
        main_tx_root[0]     = main_trap;
        init_link_root[0]   = link_trap;
    }
}
