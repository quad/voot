/*  util.c

    $Id: util.c,v 1.5 2002/06/30 09:15:06 quad Exp $

DESCRIPTION

    Yeah, everyone needs a utility module.

*/

#include "vars.h"

#include "util.h"

/* NOTE: Returns seconds since 1/1/1950. */

uint32 time (void)
{
    uint32  vals[2];

    vals[0] = *((volatile uint32 *) (0xA0710000));
    vals[1] = *((volatile uint32 *) (0xA0710004));

    return (((vals[0] & 0x0000FFFF) << 16) | (vals[1] & 0x0000FFFF));
}
