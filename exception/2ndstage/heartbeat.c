/*  heartbeat.c

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

my_pageflip pageflip_info;

void init_heartbeat(void)
{
    exception_table_entry new_exception;
    asic_lookup_table_entry new_irq;

    /* STAGE: Catch the pageflip exceptions. */
    new_exception.type = EXP_TYPE_GEN;
    new_exception.code = EXP_CODE_UBC;
    new_exception.handler = pageflip_handler;

    add_exception_handler(&new_exception);

    /* STAGE: Catch on TA_DONE ASIC interrupts. */
    new_irq.irq = EXP_CODE_INT13;
    new_irq.mask0 = ASIC_MASK0_TADONE;
    new_irq.handler = ta_handler;

    //add_asic_handler(&new_irq);
}

#ifdef COUNT_PAGEFLIP

static void count_pageflip(void)
{
    /* STAGE: Display statistic information only in the case of a new pageflip handler. */
    if (pageflip_info.spc != spc())
    {
        biudp_printf(VOOT_PACKET_TYPE_DEBUG, "Pageflip %x lasted %u", pageflip_info.spc, pageflip_info.count);

        pageflip_info.spc = spc();
        pageflip_info.count = 0;

        biudp_printf(VOOT_PACKET_TYPE_DEBUG, "Pageflip @ %x", pageflip_info.spc);
    }

    pageflip_info.count++;
}

#endif

static void* my_heartbeat(register_stack *stack, void *current_vector)
{
    static bool done_once = FALSE;

    /* STAGE: Run this section of code only once. */
    if (!done_once)
    {
        /* STAGE: !!! Check the timer chip. See if VOOT is using it. */

        /* STAGE: Enable the various codes. */
        gamedata_enable_debug();

        done_once = TRUE;
    }

#ifdef COUNT_PAGEFLIP
    /* STAGE: Pageflip statistics. */
    count_pageflip();
#endif

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
    ((asic_lookup_table_entry *) passer)->clear_irq = TRUE;

    return current_vector;

    return my_heartbeat(stack, current_vector);
}
