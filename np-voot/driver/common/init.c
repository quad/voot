/*  main.c

    $Id: init.c,v 1.7 2002/07/06 14:18:15 quad Exp $

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

#include "assert.h"

#include "init.h"

exception_handler_f old_init_handler;

static void handle_bios_vector (void)
{
    /*
        STAGE: Give the module configuration core a chance to do something
        wonky.
    */

    module_bios_vector ();
}

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

    assert (exception_add_handler (&new_exp, &old_init_handler));

    /* STAGE: Configure the UBC for breaking on PVR pageflip. */

    ubc_configure_channel (UBC_CHANNEL_A, VIDEO_FB_BUFFER, UBC_BBR_WRITE | UBC_BBR_OPERAND);

    /* STAGE: Give the module initialization core a chance for overrides. */

    module_initialize ();

    /* STAGE: Patch the c000 handler so the BIOS won't crash. */

    bios_patch_handler = handle_bios_vector;    /* NOTE: This must occur before the copy. */
    memcpy ((uint32 *) 0x8c00c000, bios_patch_base, bios_patch_end - bios_patch_base);

    /*
        STAGE: Make sure the cache is invalidated before jumping to a
        changed future.
    */

    flush_cache ();

    /* STAGE: Special BIOS reboot. Doesn't kill the DBR. */

    (*(uint32 (*)()) (*(uint32 *) 0x8c0000e0)) (-3);
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
