/*  debug.c

    $Id: debug.c,v 1.1 2002/06/12 00:38:56 quad Exp $

DESCRIPTION

    A no-frillers, very direct, debugging loader for a driver.
    
    Tell the user we're waiting, wait for some new media, and then load the
        driver.

CHANGELOG

    Wed May  8 16:24:13 PDT 2002    Scott Robinson <scott_vo@quadhome.com>
        First revision of the debugging driver.

    Thu May  9 00:06:43 PDT 2002    Scott Robinson <scott_vo@quadhome.com>
        Fixed so driver loading actually works. ;-)

    Tue May 28 09:18:35 PDT 2002    Scott Robinson <scott_vo@quadhome.com>
        Split out the boot loading functionality.

    Tue Jun 11 17:38:41 PDT 2002    Scott Robinson <scott_vo@quadhome.com>
        Fixed a bug in the warez detection logic.

*/

#include <kos.h>
#include <conio/conio.h>
#include "gdrom.h"
#include "boot.h"

#include "debug.h"

static int      warez_enable = 1;
extern uint8    romdisk[];

KOS_INIT_ROMDISK (romdisk);

int main (void)
{
    /* STAGE: If the START button is pressed, exit out of the loader completely. */

    cont_btn_callback (maple_first_controller (), CONT_START, (cont_btn_callback_t) arch_exit);

    /* STAGE: Configure the display. */

    pvr_init_defaults ();
    conio_init (CONIO_TTY_PVR, CONIO_INPUT_LINE);   /* TODO: Change this to CONIO_INPUT_NONE once KOS supports it. */

    /* STAGE: Display our welcome and copyright text. */ 

    conio_putstr (startup_msg);

    /* STAGE: Keep us looping until a valid disc is inserted. */

    for (;;)
    {
        int status;
        int disc_type;

        /* STAGE: Let the user know that we're waiting. */

        conio_putstr (insert_disc_msg);

        /* STAGE: Check and see if a new media has been inserted. */

        wait_for_gdrom ();

        /* STAGE: Check if we want to use the media, if so, start the load sequence! */

        cdrom_get_status (&status, &disc_type);

        if (!warez_enable && disc_type != CD_GDROM)
            conio_putstr (bad_disc_msg);
        else
        {
            boot_loader ();

            conio_putstr (broken_dist_msg);
        }
    }
}
