/*  main.c

DESCRIPTION

    The code first ran by the first stage loader. We quickly abandon this
    shell and hook to various parts of the system.

*/

#include "vars.h"
#include "exception-lowlevel.h"
#include "exception.h"
#include "assert.h"
#include "util.h"

#define LOADED_POINT        0x8C300000
#define REAL_LOAD_POINT     0x8C010000

void handle_bios_vector(void)
{
    assert(dbr() == exception_handler_lowlevel);
}

int32 dc_main(void)
{
    /* STAGE: Initialize the UBC. */
    *UBC_R_BBRA = *UBC_R_BBRB = 0;
    *UBC_R_BRCR = UBC_BRCR_UBDE | UBC_BRCR_PCBA | UBC_BRCR_PCBB;
    dbr_set(exception_handler_lowlevel);
    vbr_set(vbr_buffer);

    /* STAGE: Initialize both UBC channels. */
    init_ubc_a_exception();

    /* STAGE: Wait enough cycles for the UBC to be working properly. */
    ubc_wait();

    /* STAGE: Patch the c000 handler so the BIOS won't crash. */
    bios_patch_handler = handle_bios_vector;    /* This must occur before the copy. */
    memcpy((uint32 *) 0x8c00c000, bios_patch_base, bios_patch_end - bios_patch_base);

    /* STAGE: Make sure the cache is invalidated before jumping to a changed future. */
    flush_cache();

    /* STAGE: Special BIOS reboot. Doesn't kill the DBR. */
    (*(uint32 (*)()) (*(uint32 *) 0x8c0000e0)) (-3);

    /* STAGE: Freeze it in an interesting fashion. */
    assert(0);
}
