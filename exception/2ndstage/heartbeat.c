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
#include "trap.h"

#include "heartbeat.h"

void init_heartbeat(void)
{
    exception_table_entry new_exception;
    asic_lookup_table_entry new_irq;

    /* STAGE: Catch the pageflip exceptions. */
    new_exception.type = EXP_TYPE_GEN;
    new_exception.code = EXP_CODE_UBC;
    new_exception.handler = pageflip_handler;

    //add_exception_handler(&new_exception);

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

    trap_ping_perframe();

    return current_vector;
}

void* pageflip_handler(register_stack *stack, void *current_vector)
{
    /* STAGE: We only break on the pageflip (channel A) exception. */
    if (*UBC_R_BRCR & UBC_BRCR_CMFA)
    {
        /* STAGE: Be sure to clear the proper bit. */
        *UBC_R_BRCR &= ~(UBC_BRCR_CMFA);

        /* STAGE: Pass control to the actual code base. */
        return my_heartbeat(stack, current_vector);
    }
    else
        return current_vector;
}

void* ta_handler(void *passer, register_stack *stack, void *current_vector)
{
    ((asic_lookup_table_entry *) passer)->clear_irq = FALSE;

    return my_heartbeat(stack, current_vector);
}
