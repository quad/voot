/*  timer.c

    $Id: timer.c,v 1.1 2002/11/12 08:52:33 quad Exp $

DESCRIPTION

    System timer access through Katana.

*/

#include "vars.h"
#include "util.h"
#include "searchmem.h"

#include "timer.h"

static uint8       *timer_root;
static const uint8  timer_key[] = { 0x48, 0xd2, 0x00, 0xe3 };

void timer_init (void)
{
    if (!timer_root || memcmp (timer_root, timer_key, sizeof (timer_key)))
        timer_root = search_gamemem (timer_key, sizeof (timer_key));

    timer_gen_set_clock (TIMER_CLOCK_DEFAULT);
    timer_gen_set_count (0);
    timer_gen_start ();
}

uint32 timer_free_count (void)
{
    if (timer_root)
        return (*(uint32 (*)()) timer_root + TIMER_FREE_COUNT_INDEX) ();
    else
        return 0;
}

uint32 timer_free_micro (uint32 count)
{
    if (timer_root)
        return (*(uint32 (*)()) timer_root + TIMER_FREE_MICRO_INDEX) (count);
    else
        return 0;
}

void timer_gen_start (void)
{
    if (timer_root)
        return (*(void (*)()) timer_root + TIMER_GEN_START_INDEX) ();
}

void timer_gen_set_clock (uint16 clock)
{
    if (timer_root)
        return (*(void (*)()) timer_root + TIMER_GEN_SET_CLOCK_INDEX) (clock);
}

void timer_gen_set_count (uint32 count)
{
    if (timer_root)
        return (*(void (*)()) timer_root + TIMER_GEN_SET_COUNT_INDEX) (count);
}

uint32 timer_gen_count (void)
{
    if (timer_root)
        return (*(uint32 (*)()) timer_root + TIMER_GEN_COUNT_INDEX) ();
    else
        return 0;
}

uint32 timer_gen_micro (uint32 count)
{
    if (timer_root)
        return (*(uint32 (*)()) timer_root + TIMER_GEN_MICRO_INDEX) (count, TIMER_CLOCK_DEFAULT);
    else
        return 0;
}
