/*  asic.c

    $Id: asic.c,v 1.5 2002/07/06 14:18:15 quad Exp $

DESCRIPTION

    Handle the SEGA-G2 bus ASIC controller.

TODO

    Reinitialize the ASIC when reinitializing the VBR table.

*/

#include "vars.h"
#include "system.h"
#include "util.h"
#include "exception.h"

#include "asic.h"

static asic_lookup_table    asic_table;
static exception_handler_f  old_handler;

static void* asic_handle_exception (register_stack *stack, void *current_vector)
{
    uint32  index;
    uint32  code;
    void   *new_vector;

    new_vector  = current_vector;
    code        = *REG_INTEVT;

    /* STAGE: Handle exception table */

    for (index = 0; index < ASIC_TABLE_SIZE; index++)
    {
        asic_lookup_table_entry passer;

        /*
            NOTE: Technically, this can cause matchs on exceptions in cases
            where the SH4 combines them and/or the exceptions have been
            placed in a queue. However, this doesn't bother me too much.
        */

        passer.mask0        = ASIC_IRQ_STATUS[0] & asic_table.table[index].mask0;
        passer.mask1        = ASIC_IRQ_STATUS[1] & asic_table.table[index].mask1;
        passer.irq          = code;
        passer.clear_irq    = TRUE;

        if ((passer.mask0 || passer.mask1) && (asic_table.table[index].irq == passer.irq))
        {
            new_vector = asic_table.table[index].handler (&passer, stack, new_vector);

            /* STAGE: Clear the IRQ by default - but the option is controllable. */

            if (passer.clear_irq)
            {
                ASIC_IRQ_STATUS[0] = passer.mask0;
                ASIC_IRQ_STATUS[1] = passer.mask1;
            }
        }
    }

    /* STAGE: Return properly, depending if there is an older handler. */

    if (old_handler)
        return old_handler (stack, new_vector);
    else
        return new_vector;
}

bool asic_add_handler (const asic_lookup_table_entry *new_entry, asic_handler_f *parent_handler)
{
    uint32  index;

    /* STAGE: Don't allow adding of handlers until we've initialized. */

    if (!asic_table.inited)
        return FALSE;

    /* STAGE: Scan the entire ASIC table an empty slot. */

    for (index = 0; index < ASIC_TABLE_SIZE; index++)
    {
        if ((asic_table.table[index].irq == new_entry->irq) &&
            (asic_table.table[index].mask0 == new_entry->mask0) && (asic_table.table[index].mask1 == new_entry->mask1))
        {
            *parent_handler = asic_table.table[index].handler;

            asic_table.table[index].handler = new_entry->handler;

            return TRUE;
        }
        else if (!(asic_table.table[index].irq))
        {
            volatile uint32    *mask_base;

            /* STAGE: Ensure there isn't any parent handler given back. */

            *parent_handler = NULL;

            /* STAGE: Determine which ASIC IRQ bank, if any, the given mask will be enabled on. */

            switch (new_entry->irq)
            {
                case EXP_CODE_INT9 :
                    mask_base = ASIC_IRQ9_MASK;
                    break;

                case EXP_CODE_INT11 :
                    mask_base = ASIC_IRQ11_MASK;
                    break;

                case EXP_CODE_INT13 :
                    mask_base = ASIC_IRQ13_MASK;
                    break;

                case EXP_CODE_ALL :
                {
                    /* STAGE: Mask the first two ASIC banks. */

                    mask_base = ASIC_IRQ9_MASK;
                    mask_base[0] |= new_entry->mask0;
                    mask_base[1] |= new_entry->mask1;

                    mask_base = ASIC_IRQ11_MASK;
                    mask_base[0] |= new_entry->mask0;
                    mask_base[1] |= new_entry->mask1;

                    /* STAGE: Have the code further on take care of the last mask. */

                    mask_base = ASIC_IRQ13_MASK;

                    break;
                }

                /* STAGE: Assume the caller knows what the hell they're doing. */

                default :
                    mask_base = NULL;
                    break;
            }

            /* STAGE: Enable the selected G2 IRQs on the ASIC. */

            if (mask_base)
                mask_base[0] |= new_entry->mask0;
                mask_base[1] |= new_entry->mask1;

            /* STAGE: Copy the new entry into our table. */

            memcpy (&asic_table.table[index], new_entry, sizeof (asic_lookup_table_entry));

            return TRUE;
        }
    }

    return FALSE;
}

void asic_init (void)
{
    exception_table_entry new_entry;

    /* STAGE: Ensure we can't initialize ourselves twice. */

    if (asic_table.inited)
        return;

    /* STAGE: Works for all the interrupt types... */

    new_entry.type      = EXP_TYPE_INT;
    new_entry.handler   = asic_handle_exception;

    /*
        STAGE: ASIC handling of all interrupts.
    
        NOTE: In our case, the handler itself checks the exception code and
        matches it to the interrupt.
    */

    new_entry.code      = EXP_CODE_ALL;

    asic_table.inited = exception_add_handler (&new_entry, &old_handler);
}
