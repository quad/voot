/*  exception.c

    $Id: exception.c,v 1.5 2002/06/23 03:22:52 quad Exp $

DESCRIPTION

    Handle incoming exceptions in readable C-code.

TODO

    Move to using a chain system on exceptions.

    Ensure duplicate table entries cannot be added.

*/

#include "vars.h"
#include "exception-lowlevel.h"
#include "ubc.h"
#include "asic.h"
#include "util.h"
#include "init.h"

#include "exception.h"

static exception_table exp_table;

/* NOTE: Pointers to the VBR Buffer. */

static uint8   *vbr_buffer;
static uint8   *vbr_buffer_katana;

static void init_vbr_table (void)
{
    /* STAGE: INTERRUPT magic sprinkles of evil to the VOOT VBR. */

    memcpy (VBR_INT (vbr_buffer) - (interrupt_sub_handler_base - interrupt_sub_handler),
            interrupt_sub_handler,
            interrupt_sub_handler_end - interrupt_sub_handler
           );

    /* STAGE: GENERAL magic sprinkes of evil to the VOOT VBR. */

    memcpy (VBR_GEN (vbr_buffer) - (general_sub_handler_base - general_sub_handler),
            general_sub_handler,
            general_sub_handler_end - general_sub_handler
           );

    /* STAGE: Relocate the Katana VBR index - bypass our entry logic. */

    vbr_buffer_katana = vbr_buffer + (sizeof (uint16) * 4);

    /* STAGE: Flush cache after modifying application memory. */

    flush_cache ();

    /* STAGE: Notify ourselves of the change. */

    exp_table.vbr_switched = TRUE;
}

static bool is_vbr_switch_time (void)
{
    uint32  int_changed;
    uint32  gen_changed;

    /* STAGE: Check to see if our VBR hooks are still installed. */

    int_changed = memcmp (VBR_INT (vbr_buffer) - (interrupt_sub_handler_base - interrupt_sub_handler),
                          interrupt_sub_handler,
                          interrupt_sub_handler_end - interrupt_sub_handler
                         );

    gen_changed = memcmp (VBR_GEN (vbr_buffer) - (general_sub_handler_base - general_sub_handler),
                          general_sub_handler,
                          general_sub_handler_end - general_sub_handler
                         );

    /* STAGE: After enough exceptions, allow the initialization. */

    return (int_changed || gen_changed) && exp_table.ubc_exception_count >= 5;
}

uint32 exception_add_handler (const exception_table_entry *new_entry)
{
    uint32  index;

    for (index = 0; index < EXP_TABLE_SIZE; index++)
    {
        if (!(exp_table.table[index].type))
        {
            memcpy (&exp_table.table[index], new_entry, sizeof (exception_table_entry));

            return index + 1;
        }
    }

    return 0;
}

void* exception_handler (register_stack *stack)
{
    uint32  exception_code;
    uint32  index;
    void   *back_vector;
    bool    do_vbr_switch;

    vbr_buffer = vbr ();

    /* STAGE: Increase our counters and set the proper back_vectors. */

    switch (stack->exception_type)
    {
        case EXP_TYPE_GEN :
        {
            exp_table.general_exception_count++;

            exception_code = *REG_EXPEVT;

            /* STAGE: Never pass on UBC interrupts to the game. */
            if (exception_code == EXP_CODE_UBC)
            {
                exp_table.ubc_exception_count++;
                back_vector = my_exception_finish;
            }
            else
            {
                back_vector = VBR_GEN (vbr_buffer_katana);
            }

            break; 
        }

        case EXP_TYPE_CACHE :
        {
            exp_table.cache_exception_count++;

            exception_code  = *REG_EXPEVT;
            back_vector     = VBR_CACHE (vbr_buffer_katana);

            break;
        }

        case EXP_TYPE_INT :
        {
            exp_table.interrupt_exception_count++;

            exception_code  = *REG_INTEVT;
            back_vector     = VBR_INT (vbr_buffer_katana);

            break;
        }

        default :
        {
            exp_table.odd_exception_count++;

            exception_code  = EXP_CODE_BAD;
            back_vector     = my_exception_finish;

            break;
        }
    }

    /* STAGE: Is some form of initialization necessary? */

    do_vbr_switch = is_vbr_switch_time ();

    if (do_vbr_switch)
    {
        bool vbr_switched;

        /* STAGE: Save the switch status before we might reinitialize. */

        vbr_switched = exp_table.vbr_switched;

        /* STAGE: Initialize the VBR hooks. */

        init_vbr_table ();

        /* STAGE: Handle the first initialization. */

        if (!vbr_switched)
        {
            /* STAGE: Handle ASIC exceptions. */

            asic_init_handler ();

            /* STAGE: Handle the initialization core. */

            np_configure ();
        }
    }

    /* STAGE: Handle exception table */

    for (index = 0; index < EXP_TABLE_SIZE; index++)
    {
        if (exp_table.table[index].code == exception_code &&
            exp_table.table[index].type == stack->exception_type)
        {
            /* STAGE: Call the handler and use whatever hook it returns. */

            back_vector = exp_table.table[index].handler (stack, back_vector);
        }
    }

    /* STAGE: We're all done. Return however we were instructed. */

    return back_vector;
}
