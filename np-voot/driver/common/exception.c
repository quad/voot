/*  exception.c

    $Id: exception.c,v 1.7 2002/07/06 14:18:15 quad Exp $

DESCRIPTION

    Handle incoming exceptions in readable C-code.

TODO

    Ensure exceptions cannot be added to the table unless the system is
    initialized. (or some derivative, because of the delayed initialization
    system.)

*/

#include "vars.h"
#include "ubc.h"
#include "video.h"
#include "asic.h"
#include "util.h"
#include "init.h"

#include "exception.h"

static exception_table exp_table;

/* NOTE: Pointers to the VBR Buffer. */

static uint8   *vbr_buffer;
static uint8   *vbr_buffer_katana;      /* NOTE: Is only valid after exception_init () */

static bool exception_vbr_ok (void)
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

    return !(int_changed || gen_changed);
}

exception_init_e exception_init (void)
{
    /*
        STAGE: We must have already begun processing an exception to have
        initialized. The easiest method of doing this is via calling ubc_init.

        We must also have already waited a certain number of clean
        exceptions before screwing with the VBR. This is paranoia.

        TODO: Determine if the vbr is variable. If not, remove all
        vbr_buffer and vbr_buffer_katana references and replace with macros.
    */

    if (!vbr_buffer || exception_vbr_ok () || (exp_table.ubc_exception_count < 5))
        return FAIL;

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

    if (exp_table.vbr_switched)
    {
        return REINIT;
    }
    else
    {
        exp_table.vbr_switched = TRUE;

        return INIT;
    }
}


bool exception_add_handler (const exception_table_entry *new_entry, exception_handler_f *parent_handler)
{
    uint32  index;

    /* STAGE: Search the entire table for either a match or an opening. */

    for (index = 0; index < EXP_TABLE_SIZE; index++)
    {
        /*
            STAGE: If the entry is configured with same type and code as the
            new handler, push it on the stack and let's roll!
        */

        if ((exp_table.table[index].type == new_entry->type) && (exp_table.table[index].code == new_entry->code))
        {
            *parent_handler = exp_table.table[index].handler;

            exp_table.table[index].handler = new_entry->handler;

            return TRUE;
        }
        /*
            STAGE: We've reached the end of the filled entries, I guess we have to create our own.
        */
        else if (!(exp_table.table[index].type))
        {
            *parent_handler = NULL;

            memcpy (&exp_table.table[index], new_entry, sizeof (exception_table_entry));

            return TRUE;
        }
    }

    return FALSE;
}

void* exception_handler (register_stack *stack)
{
    uint32  exception_code;
    uint32  index;
    void   *back_vector;

    /* STAGE: Ensure vbr buffer is set... */

    vbr_buffer = vbr ();

    /* STAGE: Increase our counters and set the proper back_vectors. */

    switch (stack->exception_type)
    {
        case EXP_TYPE_GEN :
        {
            exp_table.general_exception_count++;

            exception_code = *REG_EXPEVT;

            /* STAGE: Never pass on UBC interrupts to the game. */

            if ((exception_code == EXP_CODE_UBC) || (exception_code == EXP_CODE_TRAP))
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

    /* STAGE: Handle exception table */

    for (index = 0; index < EXP_TABLE_SIZE; index++)
    {
        if (((exp_table.table[index].code == exception_code)        || (exp_table.table[index].code == EXP_CODE_ALL)) &&
            ((exp_table.table[index].type == stack->exception_type) || (exp_table.table[index].type == EXP_TYPE_ALL)))
        {
            /* STAGE: Call the handler and use whatever hook it returns. */

            back_vector = exp_table.table[index].handler (stack, back_vector);
        }
    }

    /* STAGE: We're all done. Return however we were instructed. */

    return back_vector;
}
