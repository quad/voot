/*  debug.c

    $Id: debug.c,v 1.5 2002/11/07 02:16:02 quad Exp $

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
#include <boot.h>
#include "gdrom.h"

#include "debug.h"

static int      warez_enable = 1;
extern uint8    romdisk[];

KOS_INIT_ROMDISK (romdisk);

void wait_for_user (void)
{
    cont_cond_t cond;

    /* STAGE: Tell the user what the hell to do... */

    conio_putstr (next_page_msg);

    /* STAGE: Clear out the buttons so our first evalution is true... */

    cond.buttons = CONT_A;

    /* STAGE: Wait until any button is pressed... */

    while (cond.buttons & CONT_A)
        cont_get_cond (maple_first_controller (), &cond);

    /* STAGE: Wait until it's released... */

    while (!(cond.buttons & CONT_A))
        cont_get_cond (maple_first_controller (), &cond);
}

int main (void)
{
    FILE   *in_desc;

    /* STAGE: If the START button is pressed, exit out of the loader completely. */

    cont_btn_callback (maple_first_controller (), CONT_START, (cont_btn_callback_t) arch_exit);

    /* STAGE: Configure the display. */

    pvr_init_defaults ();
    conio_init (CONIO_TTY_PVR, CONIO_INPUT_LINE);   /* TODO: Change this to CONIO_INPUT_NONE once KOS supports it. */
    conio_set_theme (CONIO_THEME_C64);

    /* STAGE: Display our welcome and copyright text. */ 

    conio_putstr (startup_msg);

    wait_for_user ();
    conio_clear ();
    conio_gotoxy (0, 0);

    /* STAGE: Open the description file. Parse and display it. */

    in_desc = fopen ("/rd/driver.desc", "r");

    if (in_desc)
    {
        char    line[80];

        while (fgets (line, sizeof (line), in_desc))
        {
            if (!stricmp ("<hr>\n", line))
            {
                wait_for_user ();
                conio_clear ();
                conio_gotoxy (0, 0);
            }
            else
                conio_putstr (line);
        }

        fclose (in_desc);
    }
    else
    {
        conio_putstr (no_desc_msg);

        wait_for_user ();
        conio_clear ();
        conio_gotoxy (0, 0);
    }
        

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
            boot_loader ("/rd/driver.bin");

            conio_putstr (broken_dist_msg);
        }
    }
}
