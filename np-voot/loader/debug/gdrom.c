/*  gdrom.c

    $Id: gdrom.c,v 1.1 2002/05/26 10:31:59 quad Exp $

DESCRIPTION

    This code is ripped fairly wholesale from libdream which pretty much
    ripped it wholesale from Marcus' original example code.

    Isn't it good to see code sharing taking place in the Dreamcast
    community?

CHANGELOG

    Wed May  8 16:51:53 PDT 2002    Scott Robinson <scott_vo@quadhome.com>
        Ported revision from the original "exception" version.

*/

#include <kos.h>

#include "gdrom.h"

void wait_for_gdrom (void)
{
    int status, disc_type;

    /* STAGE: Make sure the drive has been initialized. */

    cdrom_reinit ();

    /* STAGE: Wait for the drive to go empty. */

    for (;;)
    {
        cdrom_get_status (&status, &disc_type);

        if (status == CD_STATUS_OPEN)
            break;

        vid_waitvbl ();
    }

    /* STAGE: Wait for a media we support. */

    for (;;)
    {
        cdrom_get_status (&status, &disc_type);

        if (status == CD_STATUS_PAUSED && (disc_type == CD_GDROM || disc_type == CD_CDROM_XA || disc_type == CD_CDROM))
            break;

        vid_waitvbl ();
    }
}
