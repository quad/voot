/*  biosfont.c

    $Id: biosfont.c,v 1.4 2002/07/06 14:18:15 quad Exp $

DESCRIPTION

    A accessor module for the BIOS font functions.

CREDIT

    This code is an almost exact duplicate from Dan Potter's libdream's
    biosfont.c - except this version of the source checks the BIOS mutex.

TODO

    Remove optional resolutions and determine calculations from video registers.

    Attempt to cleanup and optimize bfont_draw inner loops.

*/

#include "vars.h"
#include "util.h"
#include "biosfont-lowlevel.h"

#include "biosfont.h"

static volatile uint8  *bfont_address;

void bfont_init (void)
{
    /* STAGE: Check the BIOS semaphore. */

    if (!bfont_lock ())
    {
        /* STAGE: Cache the biosfont address. */

        bfont_address = bfont_get_address ();

        bfont_unlock ();
    }
}

/* NOTE: Given a character, find it in the BIOS font if possible. */

static volatile uint8* bfont_find_char (uint32 ch)
{
    int index = -1;

    /* STAGE: 33-126 in ASCII are 1-94 in the font. */

    if (ch >= 33 && ch <= 126)
        index = ch - 32;

    /* STAGE: 160-255 in ASCII are 96-161 in the font. */

    if (ch >= 160 && ch <= 255)
        index = ch - (160 - 96);

    /* STAGE: Map anything else to a space. */

    if (index == -1)
        index = 72 << 2;

    return bfont_address + index * 36;
}

/* NOTE: Given a character, draw it into a buffer. */

bool bfont_draw (uint16 *buffer, uint32 bufwidth, uint32 c)
{
    uint8   ch[75];
    uint32  ch_base;
    uint16  word;
    uint32  x;
    uint32  y;

    if (!bfont_address)
        return FALSE;

    /* STAGE: Check the BIOS mutex. */

    if (!bfont_lock ())
    {
        memcpy (ch, (uint8 *) bfont_find_char (c), sizeof (ch));

        bfont_unlock ();
    }
    else
    {
        return FALSE;
    }

    ch_base = 0;

    for (y = 0; y < BFONT_CHAR_HEIGHT; )
    {
        /* STAGE: Draw the first row. */

        word = (((uint16) ch[ch_base + 0]) << 4) | ((ch[ch_base + 1] >> 4) & 0x0f);

        for (x = 0; x < BFONT_CHAR_WIDTH; x++)
        {
            *buffer = 0x0;

            if (word & (0x0800 >> x))
                *buffer = 0xff;

            buffer++;
        }

        buffer += bufwidth - BFONT_CHAR_WIDTH;
        y++;
        
        /* STAGE: Draw the second row. */

        word = ((((uint16) ch[ch_base + 1]) << 8) & 0xf00) | ch[ch_base + 2];

        for (x = 0; x < BFONT_CHAR_WIDTH; x++)
        {
            *buffer = 0x0;

            if (word & (0x0800 >> x))
                *buffer = 0xff;

            buffer++;
        }

        buffer += bufwidth - BFONT_CHAR_WIDTH;
        y++;
        
        ch_base += 3;
    }

    return TRUE;
}

bool bfont_draw_str (uint16 *buffer, uint32 width, const char *str)
{
    bool retval;

    retval = FALSE;

    while (*str && (retval = bfont_draw (buffer += BFONT_CHAR_WIDTH, width, *str++)));

    return retval;
}
