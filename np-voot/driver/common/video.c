/*  video.c

    $Id: video.c,v 1.1 2002/06/12 09:33:51 quad Exp $

DESCRIPTION

    A collection of framebuffer and PVR interaction functions.

*/

#include "vars.h"

#include "video.h"

/* CREDIT: Borrowed from Dan Potter's libdream. */

void video_waitvbl (void)
{
    volatile uint32 *vbl = ((volatile uint32 *) 0xa05f8000) + 0x010c / sizeof (uint32);

    while (!(*vbl & 0x01ff));
    while (*vbl & 0x01ff);
}

void video_clear (int16 r, int16 g, int16 b)
{
    uint16 *vram_s;
    uint16  pixel = (
                        ((r >> 3) << 11) |
                        ((g >> 2) << 5) |
                        ((b >> 3) << 0)
                   );
    int32   i;
    int32   vram_size;

    vram_s      = VIDEO_VRAM_START;
    vram_size   = 640 * 480;

    for (i = 0; i < vram_size; i++)
        vram_s[i] = pixel;
}
