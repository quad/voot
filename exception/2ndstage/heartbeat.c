/*  heartbeat.c

    Every pageflip we receive a call here. It's a great timer.
*/

#include "vars.h"
#include "exception.h"
#include "exception-lowlevel.h"
#include "system.h"
#include "biudp.h"
#include "gamedata.h"
#include "heartbeat.h"

void init_heartbeat(void)
{
    exception_table_entry new;

    /* STAGE: Catch the pageflip exceptions. */
    new.type = EXP_TYPE_GEN;
    new.code = EXP_CODE_UBC;
    new.handler = heartbeat;

    add_exception_handler(&new);

    /* STAGE: !!! There needs to be a better method of notification. */
}

static void count_pageflip(void)
{
    static uint32 saved_spc;
    static uint32 save_count = 0;

    if (saved_spc != spc())
    {
        biudp_write_str("[UBC] Pageflip 0x");
        biudp_write_hex(saved_spc);
        biudp_write_str(" lasted 0x");
        biudp_write_hex(save_count);
        biudp_write_str("\r\n");

        saved_spc = spc();
        save_count = 0;

        biudp_write_str("[UBC] Pageflip @ 0x");
        biudp_write_hex(saved_spc);
        biudp_write_str("\r\n");
    }

    save_count++;
}

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

    /* STAGE: Pageflip statistics. */
    count_pageflip();

    return current_vector;
}

void* heartbeat(register_stack *stack, void *current_vector)
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
