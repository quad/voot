/*  main.c

    $Id: init.c,v 1.5 2002/06/23 03:22:52 quad Exp $

DESCRIPTION

    This is the C initialization core for the np-voot common library.

    The "np_init" function is the receiver of the loader's second stage function call.

    The "handle_bios_vector" function is called by the BIOS during the boot cycle.

    The "np_configure" function is the handler of main initializationm,
    after VOOT has been loaded.

*/

#include "vars.h"
#include "ubc.h"
#include "exception-lowlevel.h"
#include "video.h"
#include "util.h"
#include "assert.h"
#include "malloc.h"
#include "biosfont.h"
#include "callbacks.h"

#include "init.h"

/*
    NOTE: We can only support four arguments. Any further would require
    stack management.
*/

void np_initialize (void *arg1, void *arg2, void *arg3, void *arg4)
{
    /* STAGE: Initialize the UBC. */

    ubc_init ();

    /* STAGE: Configure the UBC for breaking on PVR pageflip. */

    ubc_configure_channel (UBC_CHANNEL_A, VIDEO_FB_BUFFER, UBC_BBR_WRITE | UBC_BBR_OPERAND);
    dbr_set (ubc_handler_lowlevel);

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

void handle_bios_vector (void)
{
    /* STAGE: Make sure the BIOS hasn't done anything funny. */

    assert_x (dbr () == ubc_handler_lowlevel, dbr ());

    /* STAGE: It's nice to not have a corrupted BSS segment. */
 
    assert_x ((uint8 *) vbr () >= end, vbr ());

    /*
        STAGE: Give the module configuration core a chance to do something
        wonky.
    */

    module_bios_vector ();
}
