/*  main.c

    $Id: init.c,v 1.10 2002/11/14 06:09:48 quad Exp $

DESCRIPTION

    This is the C initialization core for the np-voot common library.

    The "np_initialize" function is the receiver of the loader's second stage function call.

    The "np_configure" function is the handler of main initializationm,
    after VOOT has been loaded.

*/

#include "vars.h"
#include "ubc.h"
#include "exception.h"
#include "video.h"
#include "callbacks.h"
#include "util.h"
#include "asic.h"
#include "biosfont.h"
#include "malloc.h"

#include "init.h"

static exception_handler_f  old_init_handler;
static bool                 have_initialized;

#ifdef BIOS_VECTOR_BYPASS
#warning Experimental BIOS vector code included in common library.

static uint32               old_bios_vector;

static uint32 handle_bios_vector (uint32 arg_a, uint32 arg_b, uint32 arg_c, uint32 arg_d)
{
    /*
        STAGE: Give the module configuration core a chance to do something
        wonky.
    */

    return (*(uint32 (*)()) old_bios_vector) (arg_a, arg_b, arg_c, arg_d);
}

#endif

static void* init_handler (register_stack *stack, void *current_vector)
{
    exception_init_e    init_result;

    init_result = exception_init ();

    /* STAGE: Should be (re-)configure the VBR? */

    if (init_result)
    {
        /* STAGE: Configure ASIC interrupt core. */

        asic_init ();

        /* STAGE: Handle the first initialization. */

        if (init_result == INIT)
            np_configure ();
    }
    else if (!have_initialized)
    {
        if (ubc_is_channel_break (UBC_CHANNEL_A))
        {
            /*
                STAGE: Emulate a cleaner version of the SR write which would
                otherwise disable our entire boot process.
            */

            stack->spc += 2;            /* STAGE: Skip the SR write. */
            stack->sr   = 0x600000f0;   /* STAGE: Leave the BL active. */
        }
        else if (ubc_is_channel_break (UBC_CHANNEL_B))
        {
            /* STAGE: Configure the UBC for breaking on PVR pageflip. */

            ubc_configure_channel (UBC_CHANNEL_A, VIDEO_FB_BUFFER, UBC_BBR_WRITE | UBC_BBR_OPERAND);

            /* STAGE: Clear out the second channel which got us here... */

            ubc_clear_channel (UBC_CHANNEL_B);

            /* STAGE: Make sure we don't pull this crap twice. */

            have_initialized = TRUE;

#ifdef BIOS_VECTOR_BYPASS
            /* STAGE: Redirect the Flash ROM syscall. */

            old_bios_vector = *((uint32 *) 0x8c0000b8);
            *((uint32 *) 0x8c0000b8) = (uint32) handle_bios_vector;
#else
            /* STAGE: Perform safe reset syscall. */

            (*(uint32 (*)()) (*(uint32 *) 0x8c0000e0)) (-3);
#endif
        }
    }

    /* STAGE: This code should never occur, but we're polite. */

    if (old_init_handler)
        return old_init_handler (stack, current_vector);
    else
        return current_vector;
}

/*
    NOTE: We can only support four arguments. Any further would require
    stack management.
*/

void np_initialize (void *arg1, void *arg2, void *arg3, void *arg4)
{
    exception_table_entry   new_exp;

    /* STAGE: Initialize the UBC. */

    ubc_init ();

    /* STAGE: Ensure we're ready for the UBC exception. */

    new_exp.type    = EXP_TYPE_GEN;
    new_exp.code    = EXP_CODE_UBC;
    new_exp.handler = init_handler;

    exception_add_handler (&new_exp, &old_init_handler);

    /* STAGE: Configure the UBC to break before the SR reset and after the hardware reset... */

    ubc_configure_channel (UBC_CHANNEL_A, 0xac0002d6, UBC_BBR_READ | UBC_BBR_INSTRUCT);
    ubc_configure_channel (UBC_CHANNEL_B, 0xac00037a, UBC_BBR_READ | UBC_BBR_INSTRUCT);

    /* STAGE: Give the module initialization core a chance for overrides. */

    module_initialize ();

    /*
        STAGE: Make sure the cache is invalidated before jumping to a
        changed future.
    */

    flush_cache ();

    /*
        STAGE: Call the BIOS reset which will reset the hardware.

        NOTE: The following is a quick mapping of the BIOS call vector:

        -3   - bios config and ip.bin load+exec  (0x290/0x8c000420)
        -2   - reset in all but special case     (0x278/0x8c000408)
        -1   - swirl reset                       (0x138/0x8c0002c8)
        0    - menu                              (0x690/0x8c000820)
        1    - menu                              (0x138/0x8c0002c8)
        2    - check disc                        (0x7ee/0x8c00097e)
        3    - load ip.bin                       (0x138/0x8c0002c8)

        The bios doesn't kill 0x8c000000 - 0x8c00001f and 0x8c000800 -
        0x8c00fff in its rewrite of low memory.
    */

    while ((*(int32 (*)()) (*(uint32 *) 0x8c0000e0)) (2) < 0);

    *((uint32 *) 0x8c000064) = 0x8c008000;
    (*(uint32 (*)()) (*(uint32 *) 0x8c0000e0)) (3);
}

void np_configure (void)
{
    /* STAGE: Cache the biosfont address. (REQUIRED) */

    bfont_init (); 

    /* STAGE: Locate and assign syMalloc functionality. (REQUIRED) */

    malloc_init ();

    /* STAGE: Give the module configuration core a chance to set itself up. */

    module_configure ();
}
