/*  asic.c

    $Id: asic.c,v 1.1 2002/06/11 23:31:10 quad Exp $

DESCRIPTION

    Handle the SEGA-G2 bus ASIC controller.

TODO

    Convert the code to an intelligent chaining method of recognizing the
    interrupts.

*/

#include "vars.h"
#include "system.h"
#include "util.h"
#include "exception.h"

#include "asic.h"

static asic_lookup_table    asic_table;

uint32 add_asic_handler (const asic_lookup_table_entry *new_entry)
{
    uint32  index;

    /* STAGE: Scan the entire ASIC table an empty slot. */

    for (index = 0; index < ASIC_TABLE_SIZE; index++)
    {
        if (!(asic_table.table[index].irq))
        {
            volatile uint32    *mask_base;

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

                default :
                    return 0;
            }

            mask_base[0] |= new_entry->mask0;
            mask_base[1] |= new_entry->mask1;

            memcpy (&asic_table.table[index], new_entry, sizeof (asic_lookup_table_entry));

            return index + 1;
        }
    }

    return 0;
}

void* handle_asic_exception (register_stack *stack, void *current_vector)
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

    return new_vector;
}

void init_asic_handler (void)
{
    exception_table_entry new_entry;

    /* STAGE: Works for all the interrupt types... */

    new_entry.type      = EXP_TYPE_INT;
    new_entry.handler   = handle_asic_exception;

    /* STAGE: ASIC handling of interrupt 13. */

    new_entry.code      = EXP_CODE_INT13;

    add_exception_handler (&new_entry);

    /* STAGE: ASIC handling of interrupt 11. */

    new_entry.code      = EXP_CODE_INT11;

    add_exception_handler (&new_entry);

    /* STAGE: ASIC handling of interrupt 9. */

    new_entry.code      = EXP_CODE_INT9;

    add_exception_handler (&new_entry);
}
