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
#include "customize.h"

#include "heartbeat.h"

static char vm_mode;

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
    /* STAGE: Make sure we have a valid customization break. */
    customize_reinit();

    /* STAGE: Enable the various codes. */
    gamedata_enable_debug();

    return current_vector;
}

void* ta_handler(void *passer, register_stack *stack, void *current_vector)
{
    ((asic_lookup_table_entry *) passer)->clear_irq = FALSE;

    return my_heartbeat(stack, current_vector);
}
