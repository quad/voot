/*  heartbeat.c

DESCRIPTION

    Every pageflip we receive a call here. It's a great timer.

*/

#include "vars.h"
#include "system.h"
#include "exception.h"
#include "exception-lowlevel.h"
#include "asic.h"
#include "voot.h"
#include "gamedata.h"

#include "heartbeat.h"

void init_heartbeat(void)
{
    asic_lookup_table_entry new_irq;

    /* STAGE: Catch on TA_DONE ASIC interrupts. */
    new_irq.irq = EXP_CODE_INT9;
    new_irq.mask0 = ASIC_MASK0_VSYNC;
    new_irq.handler = ta_handler;

    add_asic_handler(&new_irq);
}

static void* my_heartbeat(register_stack *stack, void *current_vector)
{
    static bool done_once = FALSE;

    /* STAGE: Run this section of code only once. */
    if (!done_once)
    {
        /* STAGE: Enable the various codes. */
        gamedata_enable_debug();

        done_once = TRUE;
    }

    return current_vector;
}

void* ta_handler(void *passer, register_stack *stack, void *current_vector)
{
    ((asic_lookup_table_entry *) passer)->clear_irq = FALSE;

    return my_heartbeat(stack, current_vector);
}
