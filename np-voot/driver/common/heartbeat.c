/*  heartbeat.c

    $Id: heartbeat.c,v 1.2 2002/06/12 10:29:01 quad Exp $

DESCRIPTION

    Every pageflip we receive a call here. It's a great timer.

TODO

    This causes random crashs. The delay on the pageflip seems to be
    harmful. I need to find a better pageflip vector.

*/

#include "vars.h"
#include "asic.h"

#include "heartbeat.h"

void heartbeat_init (void)
{
    asic_lookup_table_entry new_irq;

    /* STAGE: Catch on TA_DONE ASIC interrupts. */
    new_irq.irq     = EXP_CODE_INT9;
    new_irq.mask0   = ASIC_MASK0_VSYNC;
    new_irq.handler = ta_handler;

    add_asic_handler (&new_irq);
}

static void* my_heartbeat (register_stack *stack, void *current_vector)
{
    return module_heartbeat (stack, current_vector);
}

void* ta_handler (void *passer, register_stack *stack, void *current_vector)
{
    ((asic_lookup_table_entry *) passer)->clear_irq = FALSE;

    return my_heartbeat (stack, current_vector);
}
