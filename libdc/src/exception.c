/*  exception.c

DESCRIPTION
    Handles SH4 and ASIC SEGA-G2 bus exceptions passing them on to a ring of
    handlers.

COPYING
    See "COPYING" in the root directory of the distribution.

CHANGELOG
    Mon Aug  6 15:46:17 PDT 2001    Scott Robinson <scott_dcdev@dsn.itgo.com>
        Imported, modified, and just generally added a timestamp when I
        created the libdc distribution.

    Mon Aug  6 17:00:00 PDT 2001    Scott Robinson <scott_dcdev@dsn.itgo.com>
        Cleaned out all the weird hold-overs from the original exception.c.
        This is looking more and more like asic.h. I wonder if there is some
        way I could combine the two?

    Mon Aug  6 18:32:50 PDT 2001    Scott Robinson <scott_dcdev@dsn.itgo.com>
        Unified the ASIC and SH4 exception modules.

*/

#include "vars.h"
#include "exception.h"

uint32 add_exception_handler(exception_table *exp_table, const exception_table_entry *new_entry)
{
    uint32  index;

    for (index = 0; index < EXP_TABLE_SIZE; index++)
    {
        if(!(exp_table->table[index].code))
        {
            memcpy(&(exp_table->table[index]), new_entry, sizeof(exception_table_entry));
            return index + 1;
        }
    }

    return 0;
}

bool exception_handler(exception_table *exp_table, register_stack *stack)
{
    uint32 exception_code, index;
    bool back_vector_ok;

    back_vector_ok = TRUE;

    /* STAGE: Increase our counters and reset the proper back_vector, if nessicary */
    switch (stack->exception_type)
    {
        case EXP_TYPE_GEN:
            exp_table->general_exception_count++;
            exception_code = *REG_EXPEVT;
            if (exception_code == EXP_CODE_UBC)
            {
                exp_table->ubc_exception_count++;
                back_vector_ok = FALSE;
            }
            break; 

        case EXP_TYPE_CACHE:
            exp_table->cache_exception_count++;
            exception_code = *REG_EXPEVT;
            break;

        case EXP_TYPE_INT:
            exp_table->interrupt_exception_count++;
            exception_code = *REG_INTEVT;
            break;

        default:
            exp_table->odd_exception_count++;
            exception_code = EXP_CODE_BAD;
            back_vector_ok = FALSE;
            break;
    }

    /* STAGE: Handle exception table */
    for (index = 0; index < EXP_TABLE_SIZE; index++)
    {
        exception_table_entry passer;

        memcpy(&passer, &(exp_table->table[index]), sizeof(exception_table_entry));
        passer.mask0 &= ASIC_IRQ_STATUS[0];
        passer.mask1 &= ASIC_IRQ_STATUS[1];

        /* There are no duplicated exception types, so there is no reason to
            check that. */
        if (exp_table->table[index].code == exception_code)
        {
            /* Check if we're also matching against the ASIC. */
            if (exp_table->table[index].mask0 || exp_table->table[index].mask1)
            {
                /* No match? No ticket. */
                if (passer.mask0 || passer.mask1)
                    back_vector_ok |= exp_table->table[index].handler(&passer, stack);
            }
            else
                back_vector_ok |= exp_table->table[index].handler(&passer, stack);
        }
    }

    /* STAGE: RETURN */
    return back_vector_ok;
}
