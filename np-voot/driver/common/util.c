/*  util.c

    $Id: util.c,v 1.4 2002/06/23 03:22:52 quad Exp $

DESCRIPTION

    Yeah, everyone needs a utility module.

*/

#include "vars.h"
#include "searchmem.h"

#include "util.h"

/*
    Store Queue functions.
*/

/* CREDIT: Thanks to AndrewK - store queue accessors. */

#define QACR0   (*(volatile uint32 *) 0xff000038)
#define QACR1   (*(volatile uint32 *) 0xff00003c)

/* NOTE: Copies n bytes from src to dest, dest must be 32-byte aligned. */

void* sq_cpy (void *dest, const uint32 *src, uint32 n)
{
    uint32 *d;

    d = (uint32 *) (0xe0000000 | (((uint32) dest) & 0x03ffffc0));

    /* STAGE: Set store queue memory area as desired. */

    QACR0 = ((((uint32) dest) >> 26) << 2) & 0x1c;
    QACR1 = ((((uint32) dest) >> 26) << 2) & 0x1c;
    
    /* STAGE: Fill/write queues as many times necessary. */

    n >>= 5;

    while (n--)
    {
        d[0] = *(src++);
        d[1] = *(src++);
        d[2] = *(src++);
        d[3] = *(src++);
        d[4] = *(src++);
        d[5] = *(src++);
        d[6] = *(src++);
        d[7] = *(src++);

        asm("pref @%0" : : "r" (d));

        d += 8;
    }

    /* STAGE: Wait for both store queues to complete. */

    d       = (uint32 *) 0xe0000000;
    d[0]    = d[8]  = 0;

    return dest;
}

/* NOTE: Returns seconds since 1/1/1950. */

uint32 time (void)
{
    uint32  vals[2];

    vals[0] = *((volatile uint32 *) (0xA0710000));
    vals[1] = *((volatile uint32 *) (0xA0710004));

    return (((vals[0] & 0x0000FFFF) << 16) | (vals[1] & 0x0000FFFF));
}
