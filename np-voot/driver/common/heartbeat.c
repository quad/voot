/*  heartbeat.c

    $Id: heartbeat.c,v 1.7 2002/06/30 09:15:06 quad Exp $

DESCRIPTION

    Every VSYNC interrupt we receive a call here. It's a great timer.

TODO

    Check to ensure we aren't still crashing the Dreamcast by using this
    logic. The animation render cycle is probably a better heartbeat.

*/

#include "vars.h"
#include "asic.h"
#include "callbacks.h"

#include "heartbeat.h"

static asic_handler_f   old_vsync_handler;

static void* my_heartbeat (register_stack *stack, void *current_vector)
{
    return module_heartbeat (stack, current_vector);
}

static void* vsync_handler (void *passer, register_stack *stack, void *current_vector)
{
    void *new_vector;

    ((asic_lookup_table_entry *) passer)->clear_irq = FALSE;

    new_vector = my_heartbeat (stack, current_vector);

    if (old_vsync_handler)
        return old_vsync_handler (passer, stack, new_vector);
    else
        return new_vector;
}

void heartbeat_init (void)
{
    asic_lookup_table_entry new_irq;

    /* STAGE: Catch on TA_DONE ASIC interrupts. */

    new_irq.irq     = EXP_CODE_INT9;
    new_irq.mask0   = ASIC_MASK0_VSYNC;
    new_irq.handler = vsync_handler;

    asic_add_handler (&new_irq, &old_vsync_handler);
}
