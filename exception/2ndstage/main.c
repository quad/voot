/*  main.c

    The code first ran by the first stage loader. We quickly abandon this
    shell and hook to various parts of the system.
*/

#include <string.h>         // For memmove() - How much memory wasted?
#include "util.h"
#include "system.h"
#include "exception-lowlevel.h"
#include "exception.h"
#include "trap.h"
#include "warez_load.h"

/*
SCIF Control Register - 0xFFE80008
SCIF Receive Register - 0xFFE80014
IP.BIN/UBC Load Point - 0x8C008000
VR ? Armor Location   - 0x8CCF7402
FB Page Flip          - 0xa05f8050
*/

#define LOADED_POINT        0x8C300000
#define REAL_LOAD_POINT     0x8C010000

int dc_main(int do_descramble)
{
    unsigned long bin_size;

    /* STAGE: Initialize the UBC. */
    *UBC_R_BRCR = UBC_BRCR_UBDE;
    dbr_set(exception_handler_lowlevel);
    vbr_set(vbr_buffer);

    /* STAGE: Initialize both UBC channels. */
    init_ubc_a_exception();
    init_ubc_b_serial();

    /* STAGE: Wait enough cycles for the UBC to be working properly. */
    ubc_wait();

    /* STAGE: Handle the 1ST_READ.BIN */
    if (do_descramble)
    {
        disable_cache();
        warez_load(*((unsigned long *) REAL_LOAD_POINT));
    }
    else
    {
        /* STAGE: Relocate the 1st_read.bin */
        bin_size = *((unsigned long *) REAL_LOAD_POINT);
        memmove((unsigned char *) REAL_LOAD_POINT, (unsigned char *) LOADED_POINT, bin_size);

        /* STAGE: Execute the 1ST_READ.BIN */
        disable_cache();
        (*(void (*)()) REAL_LOAD_POINT) ();
    }

    /* Freeze the system. */
    while(1);
}
